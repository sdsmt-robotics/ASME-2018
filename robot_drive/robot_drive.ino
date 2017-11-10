#include "RoboClaw.h"
#include <SoftwareSerial.h>
#include "MatrixMath.h" 

RoboClaw roboclawOmni(&Serial1, 10000);
RoboClaw roboclawMcCanum(&Serial2, 10000);

#define address 0x80

const float RADIUS_OMNI = 1.5;      // radius of the omni wheel
const float RADIUS_MECANUM = 1.17;  // radius of the mecanum wheels
const float L_O = 3;                // distance from the omni wheel to the geometric center of the chassis

// Constant Matrix used in finding the xsi-sub-R
const float m1[9] = { 0, (-(RADIUS_MECANUM / 2)), (RADIUS_MECANUM / 2),
                      0, (RADIUS_MECANUM / 2), (RADIUS_MECANUM / 2),
                      (-(RADIUS_OMNI / L_O)), (RADIUS_OMNI / (2 * L_O)), (-(RADIUS_OMNI / (2 * L_O))) };

int vertical = A0;    // joystick's vertical output wires to the A0 pin.
int horizontal = A1;  // joystick's horizontal output wires to the A1 pin.
int verticalSensor = 0;   // holds the vertical analog output from joystick
int horizontalSensor = 0; // holds the horizontal analog output from joystick

void setup()
{
  roboclawOmni.begin(38400);
  roboclawMcCanum.begin(38400);

  pinMode(vertical, INPUT);     // sets A0 to input
  pinMode(horizontal, INPUT);   // sets A1 to input
  Serial.begin(57600);

  Matrix.Print(m1, 3, 3, "m1");   // display xsi_sub_R to serial monitor

  Serial.println();

  double phi_o = get_phi_o(25, 0);      // phi vector value for the rotational speed of the omni wheel
  double phi_m1 = get_phi_m1(25, 60);   // phi vector value for the rotational speed of the upper left mecanum wheel
  double phi_m2 = get_phi_m2(75, 80);   // phi vector value for the rotational speed of the upper right mecanum wheel

  float velocity_vector[3];   // xsi_sub_r = constant matrix (m1) * phi vector

  // calculate xsi_sub_r using the constant matrix, m1, and the phi vector
  set_vel_vector(velocity_vector, phi_o, phi_m1, phi_m2);
}

void loop()
{
  verticalSensor = analogRead(vertical);      // gets the joystick output for the vertical value 
  horizontalSensor = analogRead(horizontal);  // gets the joystick output for the horizontal value

  // maps the forward/backward and left/right movement 
  // with the correct ratio from the joystick to the motors
  int fwd_bwd = map(verticalSensor, 0, 1022, -127, 127);        
  int left_right = map(horizontalSensor, 0, 1022, -127, 127);

  // phi vector components
  double phi_o = get_phi_o(left_right, 0);
  double phi_m1 = get_phi_m1(left_right, fwd_bwd);
  double phi_m2 = get_phi_m2(left_right, fwd_bwd);

  // determines direction of omni wheel
  if (phi_o > 0)
    roboclawOmni.ForwardM1(address, phi_o);
  else roboclawOmni.BackwardM1(address, abs(phi_o));

  // determines direction of upper left mecanum wheel
  if (phi_m1 > 0)
    roboclawMcCanum.BackwardM1(address, phi_m1);
  else roboclawMcCanum.ForwardM1(address, abs(phi_m1));

  // determines direction of upper right mecanum wheel
  if (phi_m2 > 0)
    roboclawMcCanum.BackwardM2(address, phi_m2);
  else roboclawMcCanum.ForwardM2(address, abs(phi_m2));
  
// Previous attempt at movement. May still be useful
/*
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
*/
// print functions to help debug
//  Serial.print("Horizontal: ");
//  Serial.println(left_right);
//  Serial.print("Vertical: ");
//  Serial.println(fwd_bwd);
//  Serial.println(verticalSensor);
//  Serial.println(horizontalSensor);
  
  delay(100);
}

// calculates the rotational speed of the omni wheel based on a desired direction, x,
// and a desired angle, theta
 double get_phi_o(int x, int theta)
{
  double phi_o = (-((x + (L_O * theta)) / RADIUS_OMNI));
  return phi_o;
}

// calculates the rotational speed of the northwest mecanum wheel based on a 
// desired direction in x and y
double get_phi_m1(int x, int y)
{
  double phi_m1 = ((x + y) / RADIUS_MECANUM);
  return phi_m1;
}

// calculates the rotational speed of the northwest mecanum wheel based on a 
// desired direction in x and y
double get_phi_m2(int x, int y)
{
  double phi_m2 = (((-x) + y) / RADIUS_MECANUM);
  return phi_m2;
}

// caluclates xsi_sub_R using the phi vector and constant matrix
void set_vel_vector(float* velocity, float phi_o, float phi_m1, float phi_m2)
{  
  float phi_velocity[3] = { phi_o,
                            phi_m1,
                            phi_m2 };

  Matrix.Multiply(m1, phi_velocity, 3, 3, 1, velocity);                
  Matrix.Print(velocity, 3, 1, "velocity");
  Serial.println();
}
