#include <SoftwareSerial.h>
  
int joy_1a = 0;
int joy_1b = 0;
int joy_1a_old = 0;
int joy_1b_old = 0;

int joy_2a = 0;
int joy_2b = 0;
int joy_2a_old = 0;
int joy_2b_old = 0;

char map_1a = 0;
char map_1b = 0;

char map_2a = 0;
char map_2b = 0;

int button1;
bool button1p;

int button2;
bool button2p;

int button3;
bool button3p;

int button4;
bool button4p;

int button5;
bool button5p;

int button6;
bool button6p;

int button7;
bool button7p;

int button8;
bool button8p;

void setup() {
  pinMode(A0, INPUT);     // sets A0 to input
  pinMode(A1, INPUT);     // sets A1 to input
  pinMode(A2, INPUT);     // sets A2 to input
  pinMode(A3, INPUT);     // sets A3 to input
  
  pinMode(4, INPUT);
  pinMode(5, INPUT);
  pinMode(6, INPUT);
  pinMode(11, INPUT);

  pinMode(12, INPUT);
  pinMode(13, INPUT);
  pinMode(A4, INPUT);
  pinMode(A5, INPUT);

  Serial.begin(9600); //open serial port
  Serial.println("Starting Send Code");
}

void loop() {
  button1 = digitalRead(6);
  button2 = digitalRead(2);
  button3 = digitalRead(A5);
  button4 = digitalRead(3);

  button5 = digitalRead(7);
  button6 = digitalRead(5);
  button7 = digitalRead(A4);
  button8 = digitalRead(4);
  
  if (button1 == HIGH && button1p == false)
  {
      Serial.write('E');
      Serial.write('X');
      button1p = true;
  }
  if (button1== LOW && button1p == true)
  {
   button1p = false; 
  }
  
  if (button2 == HIGH && button2p == false)
  {
      Serial.write('F');
      Serial.write('X');
      button2p = true;
  }
  if (button2== LOW && button2p == true)
  {
   button2p = false; 
  }

  if (button3 == HIGH && button3p == false)
  {
      Serial.write('G');
      Serial.write('X');
      button3p = true;
  }
  if (button3== LOW && button3p == true)
  {
   button3p = false; 
  }
  
  if (button4 == HIGH && button4p == false)
  {
      Serial.write('H');
      Serial.write('X');
      button4p = true;
  }
  if (button4== LOW && button4p == true)
  {
   button4p = false; 
  }
  
  if (button5 == HIGH && button5p == false)
  {
      Serial.write('I');
      Serial.write('X');
      button5p = true;
  }
  if (button5== LOW && button5p == true)
  {
   button5p = false; 
  }
  
  if (button6 == HIGH && button6p == false)
  {
      Serial.write('J');
      Serial.write('X');
      button6p = true;
  }
  if (button6== LOW && button6p == true)
  {
   button6p = false; 
  }
  
  if (button7 == HIGH && button7p == false)
  {
      Serial.write('K');
      Serial.write('X');
      button7p = true;
  }
  if (button7== LOW && button7p == true)
  {
   button7p = false; 
  }
  
  if (button8 == HIGH && button8p == false)
  {
      Serial.write('L');
      Serial.write('X');
      button8p = true;
  }
  if (button8== LOW && button8p == true)
  {
   button8p = false; 
  }
  
  joy_1a = analogRead(A0);      // gets the joystick output for the vertical value 
  joy_1b = analogRead(A1);
  joy_2a = analogRead(A2);
  joy_2b = analogRead(A3);

  map_1a = map(joy_1a, 0, 1024, 0, 200);
  if (abs(joy_1a - joy_1a_old) > 10) //value has changed by 10
  {
    Serial.write('a');
    Serial.write(map_1a);
    Serial.write('X');
    joy_1a_old = joy_1a;
  }
  
  map_1b = map(joy_1b, 0, 1024, 0, 200);
  if (abs(joy_1b - joy_1b_old) > 10)
  {
    Serial.write('b');
    Serial.write(map_1b);
    Serial.write('X');
    joy_1b_old = joy_1b;
  }
  
  map_2a = map(joy_2a, 0, 1024, 0, 200);
  if (abs(joy_2a - joy_2a_old) > 10)
  {
    Serial.write('c');
    Serial.write(map_2a);
    Serial.write('X');
    joy_2a_old = joy_2a;
  }

  map_2b = map(joy_2b, 0, 1024, 0, 200);
  if (abs(joy_2b - joy_2b_old) > 10)
  {
    Serial.write('d');
    Serial.write(map_2b);
    Serial.write('X');
    joy_2b_old = joy_2b;
  }
  
  
  //delay(20);
  delay(50);
  //delay(1000);
}


