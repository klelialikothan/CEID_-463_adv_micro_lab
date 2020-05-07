#include <stdlib.h>

#define PIOA_ID 2
#define TC0_ID 17       
#define BUT_IDLE 0
#define BUT_PRESSED 1       

// Arduino Variables
#define continue_count  0x01
#define stop_count  0x02
#define reset_and_start_count  0x05

// AT91 Variables
unsigned int PIOA_int, TC_int;
unsigned long clk_pulse_counter;
unsigned int Channel_0_CCR;
unsigned long Channel_0_RC;

// States
unsigned int b0_state = BUT_IDLE;
unsigned int b1_state = BUT_IDLE;
unsigned int b12_state = BUT_IDLE;
unsigned int b13_state = BUT_IDLE;
unsigned int b0_prev_state = BUT_PRESSED;
unsigned int b1_prev_state = BUT_PRESSED;
unsigned int b12_prev_state = BUT_PRESSED;
unsigned int b13_prev_state = BUT_PRESSED;

// Variables
int initial = 8192;
int factor = 5;
int p1_score = 0;
int p2_score = 0;
int count_flash = 0;

bool ball_dir = true;             // true=R->L | false=L-R
bool wait_p1 = true;
bool wait_p2 = true;
bool flash_goal = false;

unsigned int data_out;
unsigned int led0 = HIGH;

void setup(){
  pinMode(19,INPUT_PULLUP);       // ODR, PUER
  pinMode(18,INPUT_PULLUP);       // ODR, PUER
  pinMode(2,OUTPUT);              // OER
  pinMode(3,OUTPUT);              // OER
  pinMode(4,OUTPUT);              // OER
  pinMode(5,OUTPUT);              // OER
  pinMode(6,OUTPUT);              // ODR
  pinMode(7,OUTPUT);              // OER
  pinMode(8,OUTPUT);              // OER
  pinMode(9,OUTPUT);              // OER
  pinMode(10,OUTPUT);             // OER
  pinMode(11,OUTPUT);             // OER
  pinMode(15,INPUT_PULLUP);       // ODR, PUER
  pinMode(14,INPUT_PULLUP);       // ODR, PUER
  
  digitalWrite(2, LOW);
  digitalWrite(3, LOW);
  digitalWrite(4, LOW);
  digitalWrite(5, LOW);
  digitalWrite(6, LOW);
  digitalWrite(7, LOW);
  digitalWrite(8, LOW);
  digitalWrite(9, LOW);
  digitalWrite(10, LOW);
  digitalWrite(11, LOW);
  
  clk_pulse_counter = 0;
  Channel_0_CCR = stop_count;
  Channel_0_RC = initial;
  PIOA_int=LOW;
  TC_int=LOW;

  Serial.begin(9600);
}

void loop(){  
  // Interrupt from TC0
  if (Channel_0_CCR == reset_and_start_count){
    clk_pulse_counter = 0;
    Channel_0_CCR = continue_count;
  }
  if (Channel_0_CCR == continue_count){
    clk_pulse_counter = clk_pulse_counter+1;
    if (clk_pulse_counter == Channel_0_RC){
        clk_pulse_counter =0;
        TC_int = HIGH;
    }
  }
  
  // Interrupt from Parallel I/O  Port Α (PIOA)
  // Interrupt from button 0
  b0_state = digitalRead(19);           
  if (b0_state != b0_prev_state){
    b0_prev_state = b0_state;
    PIOA_int = HIGH;
  }

  // Interrupt from button 1
  b1_state = digitalRead(18);         
  if (b1_state != b1_prev_state){
    b1_prev_state = b1_state;
    PIOA_int = HIGH;
  }

  // Interrupt from button 12
  b12_state = digitalRead(15);
  if (b12_state != b12_prev_state){
    b12_prev_state = b12_state;
    PIOA_int = HIGH;
  }

  // Interrupt from button 13
  b13_state = digitalRead(14);
  if (b13_state != b13_prev_state){
    b13_prev_state = b13_state;
    PIOA_int = HIGH;
  }
  
  // aic->ICCR
  if (PIOA_int | TC_int){                         
    FIQ_handler();
  }
}

void FIQ_handler(){
  data_out = (PORTB << 8) + PORTD;          // read LED states

  // Interrupt from Parallel I/O  Port Α (PIOA)
  if (PIOA_int == HIGH){                           
    PIOA_int = LOW;
    if(b0_state == BUT_IDLE){               // button 0 pressed
      b0_state = BUT_PRESSED;               // change state
      if (!count_flash){                    // if not counting goal
        if (wait_p1){                       // if player 1's turn to start
          ball_dir = true;                  // R->L direction
          digitalWrite(2, HIGH);            // ball starts at LED 0
          Channel_0_RC = initial;           // set counter to 1s
          Channel_0_CCR = 0x05;             // start counting
          wait_p1 = false;                  // game is afoot
          wait_p2 = false;                  // game is afoot
        }
        else {                              // game is ongoing (ball on the way)
          if (data_out & 0x4) {             // player 1 pass (successful defense)
            ball_dir = !ball_dir;           // ball changes direction
            Channel_0_CCR = 0x02;           // stop counting
            Channel_0_RC = initial;         // reset to 1s period
            Channel_0_CCR = 0x05;           // restart counting
          }
        }
      }
    }
    else {
      if (b0_state == BUT_PRESSED){         // button 0 released
        b0_state = BUT_IDLE;                // change button 0 state
      }
    }

    if(b1_state == BUT_IDLE){               // button 1 pressed
      b1_state = BUT_PRESSED;               // change state
      if (!count_flash){                    // if not counting goal
        if (wait_p1){                       // if player 1's turn to start
          ball_dir = true;                  // R->L direction
          digitalWrite(2, HIGH);            // ball starts at LED 0
          Channel_0_RC /= factor;           // divide period by 5 (incr frequence)
          Channel_0_CCR = 0x05;             // start counting
          wait_p1 = false;                  // game is afoot
          wait_p2 = false;                  // game is afoot
        }
        else if ((data_out & 0x4) && !ball_dir) { // game ongoing && player 1 defending
          ball_dir = true;                  // R->L direction                
          Channel_0_CCR = 0x02;             // stop counting
          Channel_0_RC /= factor;           // divide period by 5 (incr frequence)
          Channel_0_CCR = 0x05;             // restart counting
        }
        else if((data_out <= 0x40) && ball_dir){  // game ongoing && nitro
          Channel_0_CCR = 0x02;             // stop counting
          Channel_0_RC /= factor;           // divide period by 5 (incr frequence)
          Channel_0_CCR = 0x05;             // restart counting
        }
      }
    }
    else {
      if (b1_state == BUT_PRESSED){         // button 1 released
        b1_state = BUT_IDLE;                // change button 1 state
      }
    }

    if (b12_state == BUT_IDLE){             // button 12 pressed
      b12_state = BUT_PRESSED;              // change state
      if (!count_flash){                    // if not counting goal
        if (wait_p2){                       // if player 2's turn to start
          digitalWrite(11, HIGH);           // ball starts at LED 10
          Channel_0_RC = initial;           // set counter to 1s
          Channel_0_CCR = 0x05;             // start counting
          ball_dir = false;                 // L->R direction
          wait_p2 = false;                  // game is afoot
          wait_p1 = false;                  // game is afoot
        }
        else {                              // game is ongoing (ball on the way)                      
          if (data_out & 0x800) {           // player 2 pass (successful defense)
            Channel_0_CCR = 0x02;           // stop counting   
            Channel_0_RC = initial;         // reset to 1s period
            ball_dir = !ball_dir;           // ball changes direction
            Channel_0_CCR = 0x05;           // restart counting
          }
        }
      }
    }
    else {
      if (b12_state == BUT_PRESSED){        // button 12 released
        b12_state = BUT_IDLE;               // change button 12 state
      }
    }
    
    if (b13_state == BUT_IDLE){             // button 13 pressed 
      b13_state = BUT_PRESSED;              // change state
      if (!count_flash){                    // if not counting goal             
        if (wait_p2){                       // if player 2's turn to start            
          digitalWrite(11, HIGH);           // ball starts at LED 10
          Channel_0_RC /= factor;           // divide period by 5 (incr frequence)      
          Channel_0_CCR = 0x05;             // start counting     
          ball_dir = false;                 // L->R direction
          wait_p2 = false;                  // game is afoot
          wait_p1 = false;                  // game is afoot
        }
        else if ((data_out & 0x800) && ball_dir) {  // game ongoing && player 2 defending 
          Channel_0_CCR = 0x02;             // stop counting
          Channel_0_RC /= factor;           // divide period by 5 (incr frequence)
          ball_dir = false;                 // L->R direction
          Channel_0_CCR = 0x05;             // restart counting
        }
        else if ((data_out >= 0x80) && !ball_dir){  // game ongoing && nitro
          Channel_0_CCR = 0x02;             // stop counting         
          Channel_0_RC /= factor;           // divide period by 5 (incr frequence)
          Channel_0_CCR = 0x05;             // restart counting
        }
      }
    }
    else {
      if (b13_state == BUT_PRESSED){        // button 13 released
        b13_state = BUT_IDLE;               // change button 13 state
      }
    }
  }

  // TC0 interrupt
  if (TC_int == HIGH){                              
    TC_int = LOW;
    if (flash_goal){                        // if signaling goal
      if (count_flash < 12){                // if signaling not over
        // flip LED 1 state
        digitalWrite(2, led0);
        led0 = !led0;
        count_flash++;
        Channel_0_CCR = 0x05;               // restart counting
      }
      else {                                // signaling is over
        flash_goal = false;
        count_flash = 0;
        led0 = HIGH;
        Channel_0_RC = initial;             // reset timer to 1s period
        if (abs(p1_score-p2_score) == 2){   // if a player has won
          Serial.println("GAME OVER!");
          // reset variables (ready for new game)
          wait_p1 = true;
          wait_p2 = true;
          p1_score = 0;
          p2_score = 0;
        }
      }
    }
    else {                                  // game ongoing
      if (ball_dir){                        // if direction R->L
        if (data_out & 0x800){              // player 1 has scored
          p1_score++;
          ball_dir = !ball_dir;             // ball direction is reversed
          // all LEDs OFF
          data_out = 0x0;
          PORTD = data_out;
          PORTB = data_out;
          Serial.println("Player 1 SCORES!");
          Serial.print("Player 1: ");
          Serial.print(p1_score, DEC);
          Serial.print(" | Player 2: ");
          Serial.println(p2_score, DEC);
          wait_p2 = true;                   // wait for player 2 to start
          flash_goal = true;                // signal goal
          Channel_0_RC = initial/factor;    // set timer to 0.2s (5Hz)
          Channel_0_CCR = 0x05;             // start counting
        }
        else {                              // ball travelling towards player 2
          // move ball to the left by 1 LED
          data_out = data_out << 1;
          PORTD = data_out;
          PORTB = data_out >> 8;
          Channel_0_CCR = 0x05;             // restart counting
        }
      }
      else {                                // direction L->R
        if (data_out & 0x4){                // player 2 has scored
          p2_score++;
          ball_dir = !ball_dir;             // ball direction is reversed
          // all LEDs OFF
          data_out = 0x0;
          PORTD = data_out;
          PORTB = data_out;
          Serial.println("Player 2 SCORES!");
          Serial.print("Player 1: ");
          Serial.print(p1_score, DEC);
          Serial.print(" | Player 2: ");
          Serial.println(p2_score, DEC);
          wait_p1 = true;                   // wait for player 1 to start
          flash_goal = true;                // signal goal
          Channel_0_RC = initial/factor;    // set timer to 0.2s (5Hz)
          Channel_0_CCR = 0x05;             // start counting
        }
        else {                              // ball travelling towards player 1
          // move ball to the right by 1 LED
          data_out = data_out >> 1;
          PORTD = data_out;
          PORTB = data_out >> 8;
          Channel_0_CCR = 0x05;             // restart counting
        }
      }
    }
  }
}
