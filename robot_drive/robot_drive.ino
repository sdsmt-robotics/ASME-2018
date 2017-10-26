#include "RoboClaw.h"
#include <SoftwareSerial.h>

//declare motor controllers
RoboClaw roboclawOmni(&Serial1, 10000);
RoboClaw roboclawMcCanum(&Serial2, 10000);

#define address 0x80

//init variables
int vertical = A0;
int horizontal = A1;
int verticalSensor = 0;
int horizontalSensor = 0;

int right = 0;
int left = 0;

void setup()
{
  //init comms with motor controllers
  roboclawOmni.begin(38400);
  roboclawMcCanum.begin(38400);

  //currently using analog joysticks as inputs
  pinMode(vertical, INPUT);
  pinMode(horizontal, INPUT);
  
  Serial.begin(57600);
}

void loop()
{
  //read joystick inputs
  verticalSensor = analogRead(vertical);
  horizontalSensor = analogRead(horizontal);

  //map joystick values to motor controller values
  right = map(verticalSensor, 0, 1022, -127, 127);
  left = map(horizontalSensor, 0, 1022, -127, 127);

  if (left < 10 && left > -10)
  {
    //if movement can be just forwards backwards
    if (right > 10)
    {
      roboclawMcCanum.BackwardM1(address, right);
      roboclawMcCanum.BackwardM2(address, right);
      roboclawOmni.ForwardM1(address, 0);
    }
    else if (right < -10)
    {
      roboclawMcCanum.ForwardM1(address, abs(right));
      roboclawMcCanum.ForwardM2(address, abs(right));
    }
    else
    {
      roboclawMcCanum.ForwardM1(address, 0);
      roboclawMcCanum.BackwardM2(address, 0);
      roboclawOmni.ForwardM1(address, 0);
    }
  }
  else
  {
    //Can't just have forwards backwards movement
    if (right < -10 && right > 10)
    {
      //Do a point turn!
    }
    else
    {
      //stop!
      roboclawMcCanum.ForwardM1(address, 0);
      roboclawMcCanum.BackwardM2(address, 0);
      roboclawOmni.ForwardM1(address, 0);
    }
  }

  Serial.println(right);
  Serial.println(verticalSensor);
  //verticalSensor = analogRead(vertical);
  //horizontalSensor = analogRead(horizontal);
  delay(100);
}

