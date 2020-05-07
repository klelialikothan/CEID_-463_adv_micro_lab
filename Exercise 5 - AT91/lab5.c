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

void FIQ_handler (void);

PIO *pioa = NULL;
AIC *aic = NULL;
TC *tc = NULL;

// States
unsigned int b0_state = BUT_IDLE;
unsigned int b1_state = BUT_IDLE;
unsigned int b12_state = BUT_IDLE;
unsigned int b13_state = BUT_IDLE;

// Variables
int initial = 8192;
int factor = 5;
int p1_score = 0;
int p2_score = 0;
int count_flash = 0;

bool ball_dir = true;                      // true=R->L | false=L-R
bool wait_p1 = true;
bool wait_p2 = true;
bool flash_goal = false;

int main (int argc, const char*argv[] ) {
  unsigned int gen;

  STARTUP;                                 // system initialization 
  tc->Channel_0.RC = initial;              // period = 1 second
  tc->Channel_0.CMR = 2084;                // slow clock, waveform, disable CLK on RC compare
  tc->Channel_0.IDR = 0xFF;                // disable all the switches
  tc->Channel_0.IER = 0x10;                // enable RC compare

  aic->FFER = (1<<PIOA_ID) | (1<<TC0_ID);  // switches 2,17 -> FIQ
  aic->IECR = (1<<PIOA_ID) | (1<<TC0_ID);  // PIOA & TC0 interrupts enabled
  pioa->PUER  = 0x3003;                    // lines 0-1, 12-13: PULL−UP enabled
  pioa->ODR   = 0x3003;                    // lines 0-1, 12-13: input mode
  pioa->CODR  = 0xFFC;                     // lines 2-11: LOW voltage
  pioa->OER   = 0xFFC;                     // lines 2-11: output mode 

  gen         = pioa->ISR;                 // clear PIOA
  pioa->PER   = 0x3FFF;                    // lines 0-13: general purpose mode
  gen         = tc->Channel_0.SR;          // clear TC0
  aic->ICCR   = (1<<PIOA_ID)|(1<<TC0_ID);  // clear AIC
  pioa->IER   = 0x3FFF;                    // lines 0-13: interrupts enabled

  char tmp;
  while ((tmp = getchar()) != 'e') {       // Exit condition

  }

  aic->IDCR = (1<<PIOA_ID) | (1<<TC0_ID);  // AIC interrupts disabled
  tc->Channel_0.CCR = 0x02;                // timer disabled
  CLEANUP;

  return 0;
}

void FIQ_handler(void) {
  unsigned int data_in = 0;
  unsigned int fiq = 0;
  unsigned int data_out;

  fiq = aic->IPR;                         // identify interrupt source

  if (fiq & (1<<PIOA_ID)) {               // if interrupt source is PIOA
    data_in = pioa->ISR;                  // clear interrupt source
    aic->ICCR = (1<<PIOA_ID);             // clear AIC interrupt
    data_in = pioa->PDSR;                 // read input value
    data_out = pioa->ODSR;                // read output value
    data_out = data_out & 0xFFC;          // mask button states

    if (!(data_in & 0x01)) {              // button 0 pressed
      if(b0_state == BUT_IDLE) {
        b0_state = BUT_PRESSED;           // change state
        if (!count_flash){                // if not counting goal
          if (wait_p1){                   // if player 1's turn to start
            ball_dir = true;              // R->L direction
            // ball starts at LED 0
            pioa->SODR = 0x4;
            pioa->CODR = ~0x4;
            tc->Channel_0.RC = initial;   // set counter to 1s
            tc->Channel_0.CCR = 0x05;     // start counter
            wait_p1 = false;              // game is afoot
            wait_p2 = false;              // game is afoot
          }
          else {                          // game is ongoing (ball on the way)
            if (data_out & 0x4) {         // player 1 pass (successful defense) 
              ball_dir = !ball_dir;       // ball changes direction
              tc->Channel_0.CCR = 0x02;   // stop counter
              tc->Channel_0.RC = initial; // reset to 1s period
              tc->Channel_0.CCR = 0x05;   // start counter
            }
          }
        }
      }
    }
    else {
      if (b0_state == BUT_PRESSED) {      // button 0 released
        b0_state = BUT_IDLE;              // change button 0 state
      }
    }

    if (!(data_in & 0x2)) {               // button 1 pressed
      if(b1_state == BUT_IDLE) {
        b1_state = BUT_PRESSED;           // change state
        if (!count_flash){                // if not counting goal
          if (wait_p1){                   // if player 1's turn to start
            ball_dir = true;              // R->L direction
            // ball starts at LED 0
            pioa->SODR = 0x4;             
            pioa->CODR = ~0x4;
            tc->Channel_0.RC /= factor;   // divide period by 5 (incr frequence)  
            tc->Channel_0.CCR = 0x05;     // start counting
            wait_p1 = false;              // game is afoot
            wait_p2 = false;              // game is afoot
          }
          else if ((data_out & 0x4) && !ball_dir) {  // game ongoing && player 1 defending
              ball_dir = true;            // R->L direction 
              tc->Channel_0.CCR = 0x02;   // stop counting
              tc->Channel_0.RC /= factor; // divide period by 5 (incr frequence)
              tc->Channel_0.CCR = 0x05;   // restart counting
          }
          else if((data_out <= 0x40) && ball_dir){  // game ongoing && nitro
            tc->Channel_0.CCR = 0x02;     // stop counting
            tc->Channel_0.RC /= factor;   // divide period by 5 (incr frequence)
            tc->Channel_0.CCR = 0x05;     // restart counting
          }
        }
      }
    }
    else {
      if (b1_state == BUT_PRESSED) {      // button 1 released
        b1_state = BUT_IDLE;              // change button 1 state
      }
    }

    if (!(data_in & 0x1000)) {            // button 12 pressed
      if(b12_state == BUT_IDLE) {         
        b12_state = BUT_PRESSED;          // change state
        if (!count_flash){                // if not counting goal
          if (wait_p2){                   // if player 2's turn to start
            ball_dir = false;             // L->R direction
            // ball starts at LED 10
            pioa->SODR = 0x800;
            pioa->CODR = ~0x800;
            tc->Channel_0.RC = initial;   // set counter to 1s
            tc->Channel_0.CCR = 0x05;     // start counting
            wait_p1 = false;              // game is afoot
            wait_p2 = false;              // game is afoot
          }
          else {                          // game is ongoing (ball on the way)
            if (data_out & 0x800) {       // player 2 pass (successful defense)
              tc->Channel_0.CCR = 0x02;   // stop counting
              tc->Channel_0.RC = initial; // reset to 1s period
              ball_dir = !ball_dir;       // ball changes direction
              tc->Channel_0.CCR = 0x05;   // restart counting
            }
          }
        }
      }
    }
    else {
      if (b12_state == BUT_PRESSED) {     // button 12 released
        b12_state = BUT_IDLE;             // change button 12 state
      }
    }
    
    if (!(data_in & 0x2000)) {            // button 13 pressed
      if(b13_state == BUT_IDLE) {
        b132_state = BUT_PRESSED;         // change state
        if (!count_flash){                // if not counting goal              
          if (wait_p2){                   // if player 2's turn to start
            ball_dir = false;             // L->R direction
            // ball starts at LED 10
            pioa->SODR = 0x800;
            pioa->CODR = ~0x800;
            tc->Channel_0.RC /= factor;   // divide period by 5 (incr frequence)
            tc->Channel_0.CCR = 0x05;     // start counting
            wait_p2 = false;              // game is afoot
            wait_p1 = false;              // game is afoot
          }
          else if ((data_out & 0x800) && ball_dir) {  // game ongoing && player 2 defending
            tc->Channel_0.CCR = 0x02;     // stop counting
            tc->Channel_0.RC /= factor;   // divide period by 5 (incr frequence)
            ball_dir = false;             // L->R direction
            tc->Channel_0.CCR = 0x05;     // restart counting
          }
          else if ((data_out >= 0x80) && !ball_dir){  // game ongoing && nitro
            tc->Channel_0.CCR = 0x02;    // stop counting 
            tc->Channel_0.RC /= factor;  // divide period by 5 (incr frequence)
            tc->Channel_0.CCR = 0x05;    // restart counting
          }
        }
      }
    }
    else {
      if (b13_state == BUT_PRESSED) {    // button 13 released
        b13_state = BUT_IDLE;            // change button 13 state
      }
    }
  }

  if (fiq & (1<<TC0_ID)) {                    // if interrupt source is TC0
    data_out = tc->Channel_0.SR;              // clear interrupt source
    aic->ICCR = (1<<TC0_ID);                  // clear AIC interrupt
    data_out = pioa->ODSR;                    // read output value
    if (flash_goal){                          // if signaling goal
      if (count_flash < 12){                  // if signaling not over
        // flip LED 1 state
        pioa->SODR = data_out | 0x4;  
        pioa->CODR = data_out & 0x4;
        count_flash++;
        tc->Channel_0.CCR = 0x05;             // restart counting
      }
      else {                                  // signaling is over
        flash_goal = false;           
        count_flash = 0;                     
        tc->Channel_0.RC = initial;           // reset timer to 1s period
        if (abs(p1_score-p2_score) == 2){     // if a player has won
          printf("GAME OVER!");
          // reset variables (ready for new game)
          wait_p1 = true;
          wait_p2 = true;
          p1_score = 0;
          p2_score = 0;
        }
      }
    }
    else {                                    // game ongoing
      if (ball_dir){                          // if direction R->L
        if (data_out & 0x800){                // player 1 has scored
          p1_score++;
          ball_dir = !ball_dir;               // ball direction is reversed
          // all LEDs OFF
          data_out = 0x0;
          pioa->SODR = data_out;
          pioa->CODR = ~data_out;
          printf("Player 1 SCORES!\n");
          printf("Player 1: %d | Player 2: %d\n", p1_score, p2_score);
          wait_p2 = true;                     // wait for player 2 to start
          flash_goal = true;                  // signal goal
          tc->Channel_0.RC = initial/factor;  // set timer to 0.2s (5Hz)
          tc->Channel_0.CCR = 0x05;           // start counting
        }
        else {                                // ball travelling towards player 2
          // move ball to the left by 1 LED
          data_out = data_out << 1;
          pioa->SODR = data_out;
          pioa->CODR = ~data_out;
          tc->Channel_0.CCR = 0x05;           // restart counting
        }
      }
      else {                                  // direction L->R
        if (data_out & 0x4){                  // player 2 has scored
          p2_score++;                         
          ball_dir = !ball_dir;               // ball direction is reversed
          // all LEDs OFF
          data_out = 0x0;
          pioa->SODR = data_out;
          pioa->CODR = ~data_out;
          printf("Player 2 SCORES!\n");
          printf("Player 1: %d | Player 2: %d\n", p1_score, p2_score);
          wait_p1 = true;                     // wait for player 1 to start
          flash_goal = true;                  // signal goal
          tc->Channel_0.RC = initial/factor;  // set timer to 0.2s (5Hz)
          tc->Channel_0.CCR = 0x05;           // start counting
        }
        else {                                // ball travelling towards player 1
          // move ball to the right by 1 LED
          data_out = data_out >> 1;
          pioa->SODR = data_out;
          pioa->CODR = ~data_out;
          tc->Channel_0.CCR = 0x05;           // restart counting
        }
      }
    }
  }
}
