#include <SoftwareSerial.h>

SoftwareSerial xbee(2,3); //rx, tx

int joy_1a = 0;
int joy_1b = 0;

int joy_2a = 0;
int joy_2b = 0;

int map_1a = 0;
int map_1b = 0;

int map_2a = 0;
int map_2b = 0;

void setup() {
  pinMode(A0, INPUT);     // sets A0 to input
  pinMode(A1, INPUT);     // sets A0 to input
  pinMode(A2, INPUT);     // sets A0 to input
  pinMode(A3, INPUT);     // sets A0 to input

  Serial.begin(9600); //open serial port
  Serial.println("Starting Send Code");
  
  xbee.begin(9600); //open serial port
  xbee.println("SERIAL BEGIN");
}

void loop() {
  joy_1a = analogRead(A0);      // gets the joystick output for the vertical value 
  joy_1b = analogRead(A1);
  joy_2a = analogRead(A2);
  joy_2b = analogRead(A3);

  map_1a = map(joy_1a, 0, 1024, 0, 200);
  map_1b = map(joy_1b, 0, 1024, 0, 200);
  map_2a = map(joy_2a, 0, 1024, 0, 200);
  map_2b = map(joy_2b, 0, 1024, 0, 200);
  
  xbee.write('a');
  xbee.write(map_1a);
  xbee.write('X');
  Serial.print(map_1a);
  Serial.print(", ");
  delay(500);
  
  xbee.write('b');
  xbee.write(map_1b);
  xbee.write('X');
  Serial.print(map_1b);
  Serial.print(", ");
  delay(500);
  
  xbee.write('c');
  xbee.write(map_2a);
  xbee.write('X');
  Serial.print(map_2a);
  Serial.print(", ");
  delay(500);
  
  xbee.write('d');
  xbee.write(map_2b);
  xbee.write('X');
  Serial.print(map_2b);
  Serial.println();
  delay(500);
}