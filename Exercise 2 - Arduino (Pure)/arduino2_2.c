//Variables
int cycles=0;
int off_cycles=0;
int times=0;
bool push_button=false;
bool change_levels=true;

void setup(){
  pinMode(3,INPUT_PULLUP);
  pinMode(4,OUTPUT);
  pinMode(5,OUTPUT);
  pinMode(6,OUTPUT);
  pinMode(7,OUTPUT);
  pinMode(8,OUTPUT);
  pinMode(9,OUTPUT);
  pinMode(10,OUTPUT);
  
  attachInterrupt(digitalPinToInterrupt(3), cycling, RISING);
}

void loop(){
  if (push_button){
    for (times=1; times<=2; times++){
      for (cycles=1; cycles<=100; cycles++){
        if (change_levels){
          if (cycles<=100-off_cycles){
            digitalWrite(4, LOW);
            digitalWrite(5, LOW);
            digitalWrite(6, LOW);
            digitalWrite(7, LOW);
            digitalWrite(8, LOW);
            digitalWrite(9, LOW);
            digitalWrite(10, LOW);
          }
          else {
            digitalWrite(4, HIGH);
            digitalWrite(5, HIGH);
            digitalWrite(6, HIGH);
            digitalWrite(7, HIGH);
            digitalWrite(8, HIGH);
            digitalWrite(9, HIGH);
            digitalWrite(10, HIGH);
          }
        }
        else {
          cycles=100;
          times=20;
          change_levels=false;
        }
      }
    }
    if ((off_cycles+10<=100) && change_levels){
      off_cycles+=10;
    }
    else if (change_levels){
      off_cycles=0;
    }
    else if (!change_levels){
      change_levels=true;
    }
  }
  else {
    for (cycles=1; cycles<=100; cycles++){
      if (cycles<=100-off_cycles){
        digitalWrite(4, LOW);
        digitalWrite(5, LOW);
        digitalWrite(6, LOW);
        digitalWrite(7, LOW);
        digitalWrite(8, LOW);
        digitalWrite(9, LOW);
        digitalWrite(10, LOW);
      }
      else {
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

void cycling(){
  push_button=!push_button;
}
