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
#define BUT_IDLE 0
#define BUT_PRESSED 1     

void FIQ_handler (void);

PIO *pioa = NULL;
AIC *aic = NULL;

unsigned int button_state = BUT_IDLE;               // button initially idle

int main (int argc, const char*argv[] ) {
    unsigned int gen;

    STARTUP;                                        // system initialisation 

    aic->FFER   = (1<<PIOA_ID);                     // switches 2,17 -> FIQ
    aic->IECR   = (1<<PIOA_ID);                     // PIOA interrupts enabled
    pioa->CODR  = 0x7F;                             // lines 0-6: LOW voltage to init
    pioa->OER   = 0x7F;                             // lines 0-6: output mode 
    gen         = pioa->ISR;                        // clear PIOA
    pioa->PER   = 0x7F;                             // lines 0-6: general purpose mode
    aic->ICCR   = (1<<PIOA_ID);                     // clear AIC
    pioa->IER   = 0x80;                             // line 7: interrupts enabled

    int cycles=1;
    int off_cycles=0;
    int times=1;
    bool push_button=false;
    bool change_levels=true;
    char tmp;

    while ((tmp = getchar()) != 'e') {                      // exit condition
        if (push_button){
            for (times=1; times<=10; times++){
                if (push_button){
                    for (cycles=1; cycles<=100; cycles++){  
                        if (cycles<=100-off_cycles){
                          pioa->CODR  = 0x7F;               // segments On (Active Low)
                        }
                        else {
                          pioa->SODR  = 0x7F;               // segments Off
                        }
                    }
                }
                else {                                      // button was pressed during cycle
                    cycles=100;                             // exit & reset
                    times=10;                               // exit & reset
                    change_levels=false;                    // exit & reset
                }
            }
            if ((off_cycles+1<=100) && change_levels){      // off_cycles<=100
                off_cycles+=1;
            }
            else if (change_levels){                        // cycle
                off_cycles=0;
            }
            else if (!change_levels){                       // button was pressed - reset
                change_levels=true;
            }
        }
        else {                                              // retain brightness level
            for (cycles=1; cycles<=100; cycles++){          // display Segments On/Off
                if (cycles<=100-off_cycles){
                  pioa->CODR  = 0x7F;                       // segments On (Active Low)
                }
                else {
                  pioa->SODR  = 0x7F;                       // segments Off
                }
            }
        }
    }

    aic->IDCR = (1<<PIOA_ID);                               // AIC interrupts disabled
    CLEANUP;
    return 0;
}

void FIQ_handler(void) {
    unsigned int data_in = 0;
    unsigned int fiq = 0;
    unsigned int data_out;

    fiq = aic->IPR;                                 // identify interrupt source

    if (fiq & (1<<PIOA_ID)) {                       // if interrupt source is PIOA
        data_in = pioa->ISR;                        // clear interrupt source
        aic->ICCR = (1<<PIOA_ID);                   // clear AIC interrupt
        data_in = pioa->PDSR;                       // read input value
        if ((data_in & 0x80)!=0x0) {                // button pressed
            if(button_state == BUT_IDLE) {          
                button_state = BUT_PRESSED;         // change button state
            }
        }
        else {
            if (button_state == BUT_PRESSED) {      // button released
                button_state = BUT_IDLE;            // change button state
                push_button=!push_button;           // change behaviour
            }
        }
    }
}