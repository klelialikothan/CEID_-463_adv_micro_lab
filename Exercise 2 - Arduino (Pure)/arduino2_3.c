// Variables
int cycles;
int off_cycles;
int times;
int total_times;
bool first_press_button;
int misses;
bool count_guess;
int correct_guesses;

void setup(){
  pinMode(2,INPUT_PULLUP);
  pinMode(3,INPUT_PULLUP);
  pinMode(4,OUTPUT);
  pinMode(5,OUTPUT);
  pinMode(6,OUTPUT);
  pinMode(7,OUTPUT);
  pinMode(8,OUTPUT);
  pinMode(9,OUTPUT);
  pinMode(10,OUTPUT);
  
  digitalWrite(4, LOW);
  digitalWrite(5, LOW);
  digitalWrite(6, LOW);
  digitalWrite(7, LOW);
  digitalWrite(8, LOW);
  digitalWrite(9, LOW);
  digitalWrite(10, LOW);

  cycles=0;
  off_cycles=0;
  times=0;
  total_times=21;
  first_press_button=false;
  misses=0;
  count_guess=false;
  correct_guesses=0;

  Serial.begin(9600);
  
  attachInterrupt(digitalPinToInterrupt(3), cycling, RISING);
}

void loop(){
  if (first_press_button==true){
    if (count_guess){
      total_times-=1;
    }
    for (times=1; times<=total_times; times++){
      if (misses<4){
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
        //delay(2000);
        if (!count_guess){
          misses+=1;
        }
        else {
          misses=0;
          count_guess=false;
          correct_guesses+=1;
          times=total_times;
        }
      }
      else {
        first_press_button=false;
        times=total_times;
      }
      if (off_cycles+1<=100){
        off_cycles+=1;
      }
      else {
        off_cycles=0;
      }
    }
    if (!first_press_button){
      Serial.println("You LOST!!");
      Serial.print("You guessed correctly ");
      Serial.print(correct_guesses,DEC);
      Serial.println(" time(s).");
      Serial.println("Better luck next time...");
      count_guess=false;
      total_times=20;
      misses=0;
      off_cycles=0;
      correct_guesses=0;
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
  if (first_press_button==false){
    first_press_button=true;
  }
  else if (off_cycles==0 && first_press_button){
    count_guess=true;
  }
}
