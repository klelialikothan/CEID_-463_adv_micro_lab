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
int off_cycles=0;
bool first_press_button=false;
bool count_guess=false;

int main (int argc, const char*argv[] ) {
    unsigned int gen;

    STARTUP;                                        // system initialisation 

    aic->FFER   = (1<<PIOA_ID);                     // switches 2,17 -> FIQ
    aic->IECR   = (1<<PIOA_ID);                     // PIOA & TC0 interrupts enabled
    pioa->CODR  = 0x7F;                             // lines 0-6: LOW voltage to init
    pioa->OER   = 0x7F;                             // lines 0-6: output mode 
    gen         = pioa->ISR;                        // clear PIOA
    pioa->PER   = 0x7F;                             // lines 0-6: general purpose mode
    aic->ICCR   = (1<<PIOA_ID);                     // clear AIC
    pioa->IER   = 0x80;                             // line 7: interrupts enabled

    int cycles=0;
    int times=0;
    int total_times=21;
    int misses=0;
    int correct_guesses=0;
    char tmp;

    while ((tmp = getchar()) != 'e') {                        // exit condition
        if (first_press_button){                              // if game is on
            if (count_guess){                                 // a correct guess was made
                total_times-=1;
            }
            for (times=1; times<=total_times; times++){
                if (misses<4){  // 3 misses allowed, initial cycle of each level doesn't count
                    for (cycles=1; cycles<=100; cycles++){    // display Segments On/Off
                        if (cycles<=100-off_cycles){
                            pioa->CODR  = 0x7F;               // segments On (Active Low)
                        }
                        else {
                            pioa->SODR  = 0x7F;               // segments Off
                        }
                    }
                    if (!count_guess){                        // missed peak brightness
                        misses+=1;
                    }
                    else {                                    // correct guess, reset
                        misses=0;
                        count_guess=false;
                        correct_guesses+=1;
                        times=total_times;
                    }
                }
                else {                                        // player lost, exit game
                    first_press_button=false;
                    times=total_times;
                }
                if (off_cycles+1<=100){                       // off_cycles<=100
                    off_cycles+=1;
                }
                else {                                        // cycle
                    off_cycles=0;
                }
            }
            if (!first_press_button){                         // player lost
                // display stats
                printf("You LOST!!\n");
                printf("You guessed correctly %d time(s).\n",correct_guesses);
                printf("Better luck next time...\n");
                // reset stats and control variables
                count_guess=false;
                total_times=21;
                misses=0;
                off_cycles=0;
                correct_guesses=0;
            }
          }
          else {                                        // game over or not started yet
              for (cycles=1; cycles<=100; cycles++){
                if (cycles<=100-off_cycles){
                    pioa->CODR  = 0x7F;                 // segments On (Active Low)
                }
                else {
                    pioa->SODR  = 0x7F;                 // segments Off
                }
              }
          }
    }

    aic->IDCR = (1<<PIOA_ID);                           // AIC interrupts disabled
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
                button_state = BUT_PRESSED;
            }
        }
        else {
            if (button_state == BUT_PRESSED) {
                button_state = BUT_IDLE;
                if (!first_press_button){           // if game not already on
                    first_press_button=true;        // game is afoot
                }
                else if (off_cycles==0 && first_press_button){
                    count_guess=true;               // flag correct guess
                }
            }
        }
    }
}