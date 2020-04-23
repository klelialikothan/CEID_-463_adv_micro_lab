#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <header.h>

#define PIOA_ID 2
#define TC0_ID 17       
#define BUT_IDLE 0
#define BUT_PRESSED 1       
#define OPS 0        
#define OPS_HOLD 1  

void FIQ_handler (void);

PIO *pioa = NULL;
AIC *aic = NULL;
TC *tc = NULL;

unsigned int button_state = BUT_IDLE;  // button -> initially IDLE
unsigned int ops_state = OPS;          // state of operation -> initially IDLE

int tens = 0;
int ones = 0;
int cycles = 0;
int counted_tens = 0;

int main (int argc, const char*argv[] ) {

    unsigned int gen;

    STARTUP;                                            // system initialization 
    tc->Channel_0.RC = 2048;                            // period = 0.25 seconds
    tc->Channel_0.CMR = 2084;                           // slow clock, waveform, disable CLK on RC compare
    tc->Channel_0.IDR = 0xFF;                           // disable all the switches
    tc->Channel_0.IER = 0x10;                           // enable RC compare

    aic->FFER = (1<<PIOA_ID) | (1<<TC0_ID);             // switches 2,17 -> FIQ
    aic->IECR = (1<<PIOA_ID) | (1<<TC0_ID);             // PIOA & TC0 interrupts enabled
    pioa->PUER  = 0x01;                                 // line 0: PULL−UP enabled
    pioa->ODR   = 0x01;                                 // line 0: input mode
    pioa->CODR  = 0x806000‬;                             // lines 13, 14, 23: LOW voltage
    pioa->OER   = 0x‭806000‬;                             // lines 13, 14, 23: output mode 

    gen         = pioa->ISR;                            // clear PIOA
    pioa->PER   = 0x806001;                             // lines 0, 13, 14, 23: general purpose mode
    gen         = tc->Channel_0.SR;                     // clear TC0
    aic->ICCR   = (1<<PIOA_ID)|(1<<TC0_ID);             // clear AIC
    pioa->IER   = 0x01;                                 // line 0: interrupts enabled

    tc->Channel_0.CCR = 0x05;                           // start timer (0.25s)

    char tmp;                                           // stores keyboard input 
    while ((tmp = getchar()) != 'e') {                  // Exit condition

    }

    aic->IDCR = (1<<PIOA_ID) | (1<<TC0_ID);             // AIC interrupts disabled
    tc->Channel_0.CCR = 0x02;                           // timer ch0 disabled
    CLEANUP;
    return 0;

}

void FIQ_handler(void) {

    unsigned int data_in = 0;
    unsigned int fiq = 0;
    unsigned int data_out;

    fiq = aic->IPR;                                     // identify interrupt source

    if (fiq & (1<<PIOA_ID)) {                           // if interrupt source is PIOA
        data_in = pioa->ISR;                            // clear interrupt source
        aic->ICCR = (1<<PIOA_ID);                       // clear AIC interrupt
        data_in = pioa->PDSR;                           // read input value
        if (!(data_in & 0x01)) {                        // button pressed
            if(button_state == BUT_IDLE) {      
                button_state = BUT_PRESSED;             // change button state
                if (ops_state == OPS){                  // if IDLE mode         
                    ops_state = OPS_HOLD;               // change operation state
                    counted_tens = 0;                   // reset counted tens
                }
                else {                                  // if HOLD mode
                    ops_state = OPS;                    // change operation state
                    pioa->CODR  = 0x800000;             // line 23: LOW voltage
                }
            }
        }
        else {
            if (button_state == BUT_PRESSED) {          // button released
                button_state = BUT_IDLE;                // change button state
            }
        }
    }

    if (fiq & (1<<TC0_ID)) {                            // if interrupt source is timer ch0
        data_out = tc->Channel_0.SR;                    // clear interrupt source
        aic->ICCR = (1<<TC0_ID);                        // clear AIC interrupt
        // IDLE mode
        if (ops_state == OPS){
            switch(cycles){
                case 0:
                    // leds 1, 2 (lines 13, 14) blink
                    if (counted_tens < tens){
                        pioa->SODR = 0x6000‬;
                    }
                    else{
                        // led 1 (line 13) blinks every second
                        pioa->SODR = 0x2000‬;
                    }
                    cycles++;
                    break;
                case 1:
                    cycles++;
                    break;
                case 2:                                 // 0.5s passed -> turn ON LED(s)
                    // leds 1, 2 (lines 13, 14) blink
                    if (counted_tens < tens){
                        pioa->CODR = 0x6000‬;
                    }
                    else{
                        // led 1 (line 13) blinks every second
                        pioa->CODR = 0x2000‬;
                    }
                    cycles++;
                    break;
                case 3:                                 // 0.5s passed -> turn OFF LED(s)
                    if (ones == 9){                     // need to change tens digit
                        ones = 0;                       // reset ones to 0
                        if ((tens + 1) <= 5){           // less than 60s have been counted
                            tens++;                     // increment tens
                        }
                        else {                          // exactly 60s have been counted
                            tens = 0;                   // reset tens to 0
                        }
                        counted_tens = 0;               // reset
                    }
                    else {
                        ones++;                         // increment ones
                        if (counted_tens < tens){
                            counted_tens++;             // 1s has been counted
                        }
                    }
                    cycles = 0;                         // reset cycle counter
                    printf("%d%d\n",tens, ones)         // print new value
                    break;
            }   

            // if (cycles % 2 == 0)                 // when 0.5s have passed
            //  // led 1 (line 13) blinks once every second
            //  data_out = pioa->ODSR;                  // read output value
            //  pioa->SODR = data_out | 0x2000;         // line 13 flip
            //  pioa->CODR = data_out & 0x2000;         // line 13 flip
            //  // led 0 (line 14) blinks every time tens changes
            //  if (counted_tens < tens){
            //      pioa->SODR = data_out | 0x4000;     // line 14 flip
            //      pioa->CODR = data_out & 0x4000;     // line 14 flip
            //      if (cycles == 3){                   // 1s has passed
            //          counted_tens++;
            //      }
            //  }
            // if (cycles == 3){                        // 1s has passed
            //  if (ones == 9){                     // need to change tens digit
            //      ones = 0;                       // reset ones to 0
            //      if ((tens+1) <= 5){             // less than 60s have been counted
            //          tens++;                     // increment tens
            //      }
            //      else {                          // exactly 60s have been counted
            //          tens = 0;                   // reset tens to 0
            //      }
            //      counted_tens = 0;               // reset
            //  }
            //  else {
            //      ones++;                         // increment ones
            //  }
            //  cycles = 0;                         // reset counter
            // }
            // else {
            //  cycles++;                           // increment counter
            // }
            // tc->Channel_0.CCR = 0x05;                // reset timer (0.5s)
        }
        // HOLD mode
        else {
            // led 0 (line 23) blinks twice every second
            data_out = pioa->ODSR;                      // read output value
            pioa->SODR = data_out | 0x400000;           // line 23 flip
            pioa->CODR = data_out & 0x400000;           // line 23 flip
            if (cycles == 3){                           // 1s has passed
                if (ones == 9){                         // need to change tens digit
                    ones = 0;                           // reset ones to 0
                    if ((tens+1) <= 5){                 // less than 60s have been counted
                        tens++;                         // increment tens
                    }
                    else {                              // exactly 60s have been counted
                        tens = 0;                       // reset tens to 0
                    }
                }
                else {
                    ones++;                             // increment ones
                }
                cycles = 0;                             // reset counter
            }
            else {
                cycles++;                               // increment counter
            }
            // tc->Channel_0.CCR = 0x05;                // reset timer (0.5s)
        }
        tc->Channel_0.CCR = 0x05;                       // reset timer (count 0.25s)
    }   
}
