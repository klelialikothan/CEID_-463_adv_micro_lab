//Variables
int cycles;
int off_cycles;
char input;

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
  
  cycles=1;
  off_cycles=0;
  input='\0';
  
  Serial.begin(9600);
}

void loop(){
  if (Serial.available()){
    input=Serial.read();
    if (input=='u'){
      if ((off_cycles-10)>=0){
        off_cycles-=10;
      }
      else {
        off_cycles=0;
      }
    }
    else if(input=='d'){
      if ((off_cycles+10)<=100){
        off_cycles+=10;
      }
      else {
        off_cycles=100;
      }
    }
  }
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
