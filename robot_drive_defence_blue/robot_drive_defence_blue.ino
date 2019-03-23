#include "RoboClaw.h"

#define address 0x80

RoboClaw FrontMotors(&Serial2, 10000);

//Recieving byte
byte incomingByte = 0;

//Storage for bytes for decoding
byte incoming_command[2];
int queue_len = 0;

//storage for values being sent [button / joy] [value]
//97-100 are joystick values
//101-108 are button values
int current_vals[12];

int solenoid_pin = 3;

long int old_time = 0;

int axis = 0;
//Distinguishes from button / joystick commands
void check_command()
{
  //incoming_command[0] is the encoded value being converted to decimal
  if(incoming_command[0] >= 97 && incoming_command[0] <= 100) //axis a-d
  {
    axis = incoming_command[0];
    current_vals[(axis - 97)] = map(int(incoming_command[1]), 0, 200, -127, 127);
    Serial.print("Joystick ");
    Serial.print(incoming_command[0], HEX);
    Serial.print(" ");
    Serial.println(map(incoming_command[1], 0, 200, -127, 127));
    /*Serial.print("CURRENT VALS  ");
    Serial.print(current_vals[0]);
    Serial.print(" ");
    Serial.print(current_vals[1]);
    Serial.println();*/
  }
  else if (incoming_command[0] == 'G') //button 3 on controller
  {
    current_vals[4] = 1; //fire solenoid
    
    /*Serial.print("SOLENOID ");
    Serial.println(current_vals[4]);*/
  }
}
long right_drive;
long left_drive;
//houses the code for driving the robot
void drive_bot()
{
  /*
  FrontMotors.write(127+current_vals[0]);
  FrontMotors.write(127-current_vals[1]);
  */
    /*
    right_drive = pow(current_vals[0], 2);
    left_drive = pow(current_vals[1], 2);

    if(current_vals[0] <0)
    {
      right_drive *= -1;
    }
    if(current_vals[1] <0)
    {
      left_drive *= -1;
    }
    
    current_vals[0] = right_drive / 120;
    current_vals[1] = left_drive / 120;

    current_vals[0] = constrain(current_vals[0], -127, 127);
    current_vals[1] = constrain(current_vals[1], -127, 127);
    */
    if(current_vals[0] > 30) //deadzone of +/- 30
	{
      FrontMotors.ForwardM2(address, current_vals[0]);
    }
	else if (current_vals[0] < -30)
	{
      FrontMotors.BackwardM2(address, abs(current_vals[0]));
    }
	else
	{
      FrontMotors.ForwardM2(address, 0);
    }
    /*if(current_vals[1] > 0)
      FrontMotors.ForwardM1(address, current_vals[1]);
    else FrontMotors.BackwardM1(address, abs(current_vals[1]));*/

    if(current_vals[1] > 30)
	{
      FrontMotors.BackwardM1(address, current_vals[1]);
	}
    else if (current_vals[1] < -30)
	{
      FrontMotors.ForwardM1(address, abs(current_vals[1]));
	}
    else
	{
      FrontMotors.ForwardM1(address, 0);
	}

    if (current_vals[4] == 1)
    {
      analogWrite(solenoid_pin, 224);
      old_time = millis();
      Serial.println("FIRE SOLENOID");
    }
}
void setup() {
  Serial1.begin(9600);
  Serial.begin(9600);

  //init comms with motor controllers
  FrontMotors.begin(38400);

  Serial.println("Starting Recieve Code");

  analogWrite(solenoid_pin, 200);
  delay(500);
  analogWrite(solenoid_pin, 0);
  delay(500);
  analogWrite(solenoid_pin, 200);
  delay(500);
  analogWrite(solenoid_pin, 0);
}

void loop() {
  if (millis() - old_time > 100)
  {
    analogWrite(solenoid_pin, 0);
    current_vals[4] = 0;
  }
  
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

