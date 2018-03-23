#include "RoboClaw.h"
#include <SoftwareSerial.h>

#define address 0x80

RoboClaw FrontMotors(&Serial2, 10000);
RoboClaw BackMotors(&Serial3, 10000);

SoftwareSerial smcSerial1 = SoftwareSerial(3, 4);
SoftwareSerial smcSerial2 = SoftwareSerial(5, 6);

//Recieving byte
byte incomingByte = 0;

//Storage for bytes for decoding
byte incoming_command[2];
int queue_len = 0;

//storage for values being sent [button / joy] [value]
//97-100 are joystick values
//101-108 are button values
int current_vals[12];

int axis = 0;

// required to allow motors to move
// must be called when controller restarts and after any error
void exitSafeStart()
{
  smcSerial1.write(0x83);
  smcSerial2.write(0x83);
}
 
// speed should be a number from -3200 to 3200
void setMotorSpeed(int speed)
{
  if (speed < 0)
  {
    smcSerial1.write(0x86);  // motor reverse command
    smcSerial2.write(0x86);  // motor reverse command
    speed = -speed;  // make speed positive
  }
  else
  {
    smcSerial1.write(0x85);  // motor forward command
    smcSerial2.write(0x85);  // motor forward command
  }
  smcSerial1.write(speed & 0x1F);
  smcSerial2.write(speed & 0x1F);
  
  smcSerial1.write(speed >> 5);
  smcSerial2.write(speed >> 5);
}

//Distinguishes from button / joystick commands
void check_command()
{
  //incoming_command[0] is the encoded value being converted to decimal
  if(incoming_command[0] <= 100 && incoming_command[0] >= 97)
  {
    axis = incoming_command[0];
    current_vals[(axis - 97)] = map(int(incoming_command[1]), 0, 200, -127, 127);
    Serial.print("Joystick ");
    Serial.print(incoming_command[0], HEX);
    Serial.print(" ");
    Serial.print(map(incoming_command[1], 0, 200, -127, 127));
    Serial.println();
  }
  else if (incoming_command[0] == 70)
  {
    Serial.println("Stopping Shooter");
    setMotorSpeed(0);
  }
  else if (incoming_command[0] == 71)
  {
    Serial.println("Starting Shooter");
    setMotorSpeed(800);
  }
  else
  {
    //Serial.println(incoming_command[0]);
  }
}

int drive;
int strafe;
int rotate;

int front_left;
int rear_left;
int front_right;
int rear_right;
      
int FORWARD = 0;
int NEUTRAL = 0;

//houses the code for driving the robot
void drive_bot()
{
	
  // Determines direction and speed of each motor
    if(front_left > 0)
      FrontMotors.ForwardM1(address, front_left);
    else FrontMotors.BackwardM1(address, abs(front_left));

    if(front_right > 0)
      FrontMotors.ForwardM2(address, front_right);
    else FrontMotors.BackwardM2(address, abs(front_right));

    if(rear_left > 0)
      BackMotors.ForwardM1(address, rear_left);
    else BackMotors.BackwardM1(address, abs(rear_left));

    if(rear_right > 0)
      BackMotors.ForwardM2(address, rear_right);
    else BackMotors.BackwardM2(address, abs(rear_right));
}
void setup() {
  Serial1.begin(9600);
  Serial.begin(9600);

  //init comms with motor controllers
  FrontMotors.begin(38400);
  BackMotors.begin(38400);

    // initialize software serial object with baud rate of 19.2 kbps
  smcSerial1.begin(19200);
  smcSerial2.begin(19200);
  
   delay(5);
   
     smcSerial1.write(0xAA);  // send baud-indicator byte
  smcSerial2.write(0xAA);  // send baud-indicator byte
  
  exitSafeStart();  // clear the safe-start violation and let the motor run
  
  Serial.println("Starting Recieve Code");
}

void loop() {
  if (Serial1.available() > 0)
  {
    incomingByte = Serial1.read();
    if (incomingByte == 0x58)
    {
      Serial.println();
      check_command();
      drive_bot();
      queue_len = 0;
    } 
    else 
    {
      if(queue_len >= 2)
      {
        queue_len = 0;
      }
      incoming_command[queue_len] = incomingByte;
      queue_len++;
      Serial.print(incomingByte, HEX);
      Serial.print(" ");
    }
    //Serial.println(incomingByte, HEX);
  }

}

