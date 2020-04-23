#define PIOA_ID 2
#define TC0_ID 17       
#define BUT_IDLE 0
#define BUT_PRESSED 1       
#define OPS 0        
#define OPS_HOLD 1  

// Arduino special
#define continue_count  0x01
#define stop_count  0x02
#define reset_and_start_count  0x05

// AT91 Variables
unsigned int PIOA_int, TC_int;
unsigned long clk_pulse_counter;
unsigned int Channel_0_CCR;
unsigned int previous_button_state;
unsigned long Channel_0_RC;

// States
unsigned int button_state;
unsigned int ops_state = OPS;    // state of operation -> initially IDLE

// Variables
int tens;
int ones;
int cycles;
int counted_tens;
int elaplsed_cycles;
bool count_elapsed;

void setup(){
  pinMode(2,INPUT_PULLUP);    // ODR, PUER
  pinMode(3,OUTPUT);          // OER
  pinMode(4,OUTPUT);          // OER
  pinMode(5,OUTPUT);          // OER
  
  digitalWrite(3, LOW);
  digitalWrite(4, LOW);
  digitalWrite(5, LOW);
  
  Serial.begin(9600);
  
  tens = 0;
  ones = 0;
  cycles = 0;
  counted_tens = 0;
  elaplsed_cycles = 0;
  count_elapsed = false;
  
  clk_pulse_counter=0;
  Channel_0_CCR = continue_count;
  previous_button_state = BUT_PRESSED;
  Channel_0_RC = 2048;
  PIOA_int=LOW;
  TC_int=LOW;
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
  button_state = digitalRead(2);
  if (button_state != previous_button_state){
    previous_button_state = button_state;
    PIOA_int = HIGH;
  }
  
  // aic->ICCR
  if (PIOA_int | TC_int){                         
    FIQ_handler();
  }
}

void FIQ_handler(){
  // PIOA interrupt
  if (PIOA_int == HIGH){                            
    PIOA_int = LOW;
    if(button_state == BUT_IDLE){    
      button_state = BUT_PRESSED;                   // change button state
      count_elapsed = true;                         // start counting cycles
      if (ops_state == OPS){                        // if IDLE mode     
        ops_state = OPS_HOLD;                       // change operation state
        counted_tens = 0;                           // reset counted tens
      }
      else {                                        // if HOLD mode
        ops_state = OPS;                            // change operation state
        digitalWrite(3, LOW);                       // led 1 OFF
        digitalWrite(4, LOW);                       // led 2 OFF
        digitalWrite(5, LOW);                       // led 3 OFF
      }
    }
    else {
      if (button_state == BUT_PRESSED){             // button released
        button_state = BUT_IDLE;                    // change button state
        count_elapsed = false;                      // stop counting cycles
        if (elaplsed_cycles>=4){                    // reset           
          // reset variables
          tens = 0;
          ones = 0;
          cycles = 0;
          counted_tens = 0;
          Channel_0_CCR = 0x05;                     // reset timer (0.25s)
        }
        elaplsed_cycles = 0;
      }
    }
  }

  // TC0 interrupt
  if (TC_int == HIGH){                              
    TC_int = LOW;
    int led;
    // IDLE mode
    if (ops_state == OPS){
      switch(cycles){
        case 0:
          // led 1 (line 13) blinks every second
          digitalWrite(3, HIGH);                    // led 1 ON
          // led 2 (line 14) blinks
          if (counted_tens < tens){
            digitalWrite(4, HIGH);                  // led 2 ON
          }
          cycles++;
          break;
        case 1:
          cycles++;
          break;
        case 2:
          // led 1 (line 13) blinks every second
          digitalWrite(3, LOW);                     // led 1 ON
          // led 2 (line 14) blinks
          if (counted_tens < tens){
            digitalWrite(4, LOW);                   // led 2 ON
          }
          cycles++;
          break;
        case 3:
          if (ones == 9){                           // need to change tens digit
            ones = 0;                               // reset ones to 0
            if ((tens + 1) <= 5){                   // less than 60s have been counted
              tens++;                               // increment tens
            }
            else {                                  // exactly 60s have been counted
              tens = 0;                             // reset tens to 0
            }
            counted_tens = 0;                       // reset
          }
          else {
            ones++;                                 // increment ones
            if (counted_tens < tens){
              counted_tens++;                       // 1s has been counted
            }
          }
          cycles = 0;                               // reset counter
          // print new value
          Serial.print(tens, DEC);
          Serial.println(ones, DEC); 
          break;
      }

    }
    // HOLD mode
    else {
    // led 0 blinks twice every second
      led = digitalRead(5);                         // read output value
      digitalWrite(5, !led);                        // led 0 flip
      if (cycles == 3){                             // 1s has passed
        if (ones == 9){                             // need to change tens digit
          ones = 0;                                 // reset ones to 0
          if ((tens + 1) <= 5){                     // less than 60s have been counted
            tens++;                                 // increment tens
          }
          else {                                    // exactly 60s have been counted
            tens = 0;                               // reset tens to 0
          }
        }
        else {
          ones++;                                   // increment ones
        }
        cycles = 0;                                 // reset counter
      }
      else {
        cycles++;                                   // increment counter
      }
    }
    if (count_elapsed){                             // button has been pressed for 0.25s
      elaplsed_cycles += 1;
    }
    Channel_0_CCR = 0x05;                           // reset timer (0.25s)
  }
}
