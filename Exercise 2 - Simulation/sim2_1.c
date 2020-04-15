// Variables
int cycles;
int off_cycles;
char input;

void setup(){
  pinMode(4,OUTPUT);          // OER
  pinMode(5,OUTPUT);          // OER
  pinMode(6,OUTPUT);          // OER
  pinMode(7,OUTPUT);          // OER
  pinMode(8,OUTPUT);          // OER
  pinMode(9,OUTPUT);          // OER
  pinMode(10,OUTPUT);         // OER
  
  cycles=0;
  off_cycles=0;
  input='\0';

  Serial.begin(9600);
}

void loop(){
  if (Serial.available()){                // if serial input is available
    input=Serial.read();                  // read char from keyboard input
    if (input=='u'){                      // increase ratio
      if ((off_cycles-1)>=0){             // off_cycles>=0 
        off_cycles-=1;                    // adjust for next cycle
      }
      else {
        off_cycles=0;                     // retain peak brightness
      }
    }
    else if(input=='d'){                  // decrease ratio
      if ((off_cycles+1)<=100){           // off_cycles<=100
        off_cycles+=1;                    // adjust for next cycle
      }
      else {
        off_cycles=100;                   // remain at 0% brightness
      }
    }
  }
  for (cycles=1; cycles<=100; cycles++){  // display segments On/Off
    if (cycles<=100-off_cycles){          // segments On (Active Low)
      digitalWrite(4, LOW);
      digitalWrite(5, LOW);
      digitalWrite(6, LOW);
      digitalWrite(7, LOW);
      digitalWrite(8, LOW);
      digitalWrite(9, LOW);
      digitalWrite(10, LOW);
    }
    else {                                // segments Off
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