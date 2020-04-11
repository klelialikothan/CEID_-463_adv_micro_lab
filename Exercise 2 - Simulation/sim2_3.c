#define PIOA_ID 2       
#define BUT_IDLE 0
#define BUT_PRESSED 1 

// Variables
int cycles;
int off_cycles;
int times;
int total_times;
bool first_press_button;
int misses;
bool count_guess;
int correct_guesses;

// AT91 Variables
unsigned int previous_button_state;
unsigned int button_state;
unsigned int PIOA_int;

void setup(){
  pinMode(3,INPUT_PULLUP);    // ODR, PUER
  pinMode(4,OUTPUT);          // OER
  pinMode(5,OUTPUT);          // OER
  pinMode(6,OUTPUT);          // OER
  pinMode(7,OUTPUT);          // OER
  pinMode(8,OUTPUT);          // OER
  pinMode(9,OUTPUT);          // OER
  pinMode(10,OUTPUT);         // OER
  
  Serial.begin(9600);
  
  cycles=0;
  off_cycles=0;
  times=0;
  total_times=21;
  first_press_button=false;
  misses=0;
  count_guess=false;
  correct_guesses=0;
  previous_button_state=BUT_PRESSED;
  PIOA_int=LOW;
}

void loop(){
  // Interrupt from Parallel I/O Α (PIOA)
  button_state = digitalRead(3);
  if (button_state != previous_button_state){
    previous_button_state = button_state;
    PIOA_int = HIGH;
  }
  if (PIOA_int){                                    // aic->ICCR
    FIQ_handler();
  }
  if (first_press_button){                          // if game is on
    if (count_guess){                               // a correct guess was made
      total_times-=1;
    }
    for (times=1; times<=total_times; times++){
      if (misses<4){  // 3 misses allowed, initial cycle of each level doesn't count
        for (cycles=1; cycles<=100; cycles++){      // display Segments On/Off
          if (cycles<=100-off_cycles){              // segments On (Active Low)
            digitalWrite(4, LOW);
            digitalWrite(5, LOW);
            digitalWrite(6, LOW);
            digitalWrite(7, LOW);
            digitalWrite(8, LOW);
            digitalWrite(9, LOW);
            digitalWrite(10, LOW);
          }
          else {                                    // segments Off
            digitalWrite(4, HIGH);
            digitalWrite(5, HIGH);
            digitalWrite(6, HIGH);
            digitalWrite(7, HIGH);
            digitalWrite(8, HIGH);
            digitalWrite(9, HIGH);
            digitalWrite(10, HIGH);
          }
        }
        if (!count_guess){                          // missed peak brightness
          misses+=1;
        }
        else {                                      // correct guess, reset
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
    if (!first_press_button){                       // player lost
      // display stats
      Serial.println("You LOST!!");
      Serial.print("You guessed correctly ");
      Serial.print(correct_guesses,DEC);
      Serial.println(" time(s).");
      Serial.println("Better luck next time...");
      // reset stats and control variables
      count_guess=false;
      total_times=21;
      misses=0;
      off_cycles=0;
      correct_guesses=0;
    }
  }
  else {                                            // game over or not started yet
    for (cycles=1; cycles<=100; cycles++){
      if (cycles<=100-off_cycles){                  // segments On (Active Low)      
        digitalWrite(4, LOW);
        digitalWrite(5, LOW);
        digitalWrite(6, LOW);
        digitalWrite(7, LOW);
        digitalWrite(8, LOW);
        digitalWrite(9, LOW);
        digitalWrite(10, LOW);
      }
      else {                                        // segments Off
        digitalWrite(4, HIGH);
        digitalWrite(5, HIGH);
        digitalWrite(6, HIGH);
        digitalWrite(7, HIGH);
        digitalWrite(8, HIGH);
        digitalWrite(9, HIGH);
        digitalWrite(10, HIGH);
      }
    }
  }
}

void FIQ_handler(){
  if (PIOA_int == HIGH){                          // PIOA interrupt
    PIOA_int = LOW;
  }
  if(button_state == BUT_IDLE) {                  // button pressed   
    button_state = BUT_PRESSED;
  }
  else {
    if (button_state == BUT_PRESSED) {
    button_state = BUT_IDLE;
      if (!first_press_button){                   // if game not already on
        first_press_button=true;                  // game is afoot
      }
      else if (off_cycles==0 && first_press_button){
        count_guess=true;                         // flag correct guess
      }
    }
  }
}