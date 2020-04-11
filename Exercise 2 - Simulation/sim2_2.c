#define PIOA_ID 2       
#define BUT_IDLE 0
#define BUT_PRESSED 1 

// Variables
int cycles;
int off_cycles;
int times;
bool push_button;

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
  
  cycles=0;
  off_cycles=0;
  times=0;
  push_button=false;
  previous_button_state=BUT_PRESSED;
  PIOA_int=LOW;
}

void loop(){
  // Interrupt from Parallel I/O  Port Α (PIOA)
  button_state = digitalRead(3);
  if (button_state != previous_button_state){
    previous_button_state = button_state;
    PIOA_int = HIGH;
  }
  if (PIOA_int){                                  // aic->ICCR
    FIQ_handler();
  }
  if (push_button){
    for (times=1; times<=10; times++){
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
    }
    if (off_cycles+1<=100){                       // off_cycles<=100                
      off_cycles+=1;
    }
    else {                                        // cycle       
      off_cycles=0;
    }
  }
  else {                                          // retain brightness level
    for (cycles=1; cycles<=100; cycles++){
      if (cycles<=100-off_cycles){                // segments On (Active Low)
        digitalWrite(4, LOW);
        digitalWrite(5, LOW);
        digitalWrite(6, LOW);
        digitalWrite(7, LOW);
        digitalWrite(8, LOW);
        digitalWrite(9, LOW);
        digitalWrite(10, LOW);
      }
      else {                                      // segments Off
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
  if (PIOA_int == HIGH){                  // PIOA interrupt
    PIOA_int = LOW;
  }
  if(button_state == BUT_IDLE) {          // button pressed   
    button_state = BUT_PRESSED;
  }
  else {
    if (button_state == BUT_PRESSED) {
      button_state = BUT_IDLE;
      push_button=!push_button;           // change behaviour
    }
  }
}
