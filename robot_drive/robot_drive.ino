#include "RoboClaw.h"
#include <SoftwareSerial.h>

RoboClaw roboclawOmni(&Serial1, 10000);
RoboClaw roboclawMcCanum(&Serial2, 10000);

#define address 0x80

const int MAX_SPEED;

int vertical = A0;    // joystick's vertical output wires to the A0 pin.
int horizontal = A1;  // joystick's horizontal output wires to the A1 pin.
int verticalSensor = 0;   // holds the vertical analog output from joystick
int horizontalSensor = 0; // holds the horizontal analog output from joystick

int left_joystick = 0;

void setup()
{
  roboclawOmni.begin(38400);
  roboclawMcCanum.begin(38400);

  pinMode(vertical, INPUT);     // sets A0 to input
  pinMode(horizontal, INPUT);   // sets A1 to input
  Serial.begin(57600);
}

void loop()
{
  verticalSensor = analogRead(vertical);      // gets the joystick output for the vertical value 
  horizontalSensor = analogRead(horizontal);  // gets the joystick output for the horizontal value

  // maps the forward/backward and left/right movement 
  // with the correct ratio from the joystick to the motors
  int fwd_bwd = map(verticalSensor, 0, 1022, -127, 127);        
  int left_right = map(horizontalSensor, 0, 1022, -127, 127);


  if (left_right > 10 || left_right < -10 && fwd_bwd > 10 || fwd_bwd < -10)
  {
    // 1st Quadrant
    if (fwd_bwd > 0 && left_right > 0 && left_right > fwd_bwd)
    {
      roboclawMcCanum.ForwardM1(address, (MAX_SPEED * ratio(fwd_bwd, left_right)));
      roboclawMcCanum.BackwardM2(address, MAX_SPEED);
    }

    // 2th Quadrant
    if (fwd_bwd > 0 && left_right > 0 && left_right < fwd_bwd)
    {
      roboclawMcCanum.BackwardM1(address, (MAX_SPEED * ratio(fwd_bwd, left_right)));
      roboclawMcCanum.BackwardM2(address, MAX_SPEED);
    }
    
    // 3nd Quadrant
    if (fwd_bwd > 0 && left_right < 0 && abs(left_right) < fwd_bwd)
    {
      roboclawMcCanum.BackwardM1(address, fwd_bwd);
      roboclawMcCanum.BackwardM2(address, abs(left_right));
    }

    // 4th Quadrant
    if (fwd_bwd > 0 && left_right < 0 && abs(left_right) > fwd_bwd)
    {
      roboclawMcCanum.BackwardM1(address, fwd_bwd);
      roboclawMcCanum.ForwardM2(address, abs(left_right));
    }
    
    // 5rd Quadrant
    if (fwd_bwd < 0 && left_right < 0 && abs(left_right) > abs(fwd_bwd))
    {
      roboclawMcCanum.BackwardM1(address, abs(fwd_bwd));
      roboclawMcCanum.ForwardM2(address, abs(left_right));
      // roboclawOmni.BackwardM1(address, left_right);
    }
     
    // 6th Quadrant
    if (fwd_bwd < 0 && left_right < 0 && abs(left_right) < abs(fwd_bwd))
    {
      roboclawMcCanum.ForwardM1(address, abs(left_right));
      roboclawMcCanum.ForwardM2(address, abs(fwd_bwd));
    }  
        
    // 7th Quadrant
    if (fwd_bwd < 0 && left_right > 0 && left_right < abs(fwd_bwd))
    {
      roboclawMcCanum.ForwardM1(address, left_right);
      roboclawMcCanum.ForwardM2(address, abs(fwd_bwd));
      //roboclawOmni.ForwardM1(address, abs(left_right));
    }
      
    // 8th Quadrant
    if (fwd_bwd < 0 && left_right > 0 && left_right > abs(fwd_bwd))
    {
      roboclawMcCanum.ForwardM1(address, left_right);
      roboclawMcCanum.BackwardM2(address, abs(fwd_bwd));
    }
  }
  // sit still
  else
  {
      roboclawMcCanum.ForwardM1(address, 0);
      roboclawMcCanum.BackwardM2(address, 0);
      roboclawOmni.ForwardM1(address, 0);
  }

  Serial.print("Horizontal: ");
  Serial.println(left_right);
  Serial.print("Vertical: ");
  Serial.println(fwd_bwd);
//  Serial.println(verticalSensor);
//  Serial.println(horizontalSensor);
  
  delay(100);
}

// Calculates the ratio needed to apply to mecanum wheels for holonomic motion
float ratio(int f_b, int l_r)
{
  float ratio1 = (2 * (f_b / (sqrt(f_b / l_r))));
  Serial.println(ratio1);
  return (abs(ratio1));
}

