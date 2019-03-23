#include "RoboClaw.h"

#define address 0x80

RoboClaw FrontMotors(&Serial2, 10000); //right motors
RoboClaw BackMotors(&Serial3, 10000);

//Recieving byte
byte incomingByte = 0;

//Storage for bytes for decoding
byte incoming_command[2];
int queue_len = 0;

//storage for values being sent [button / joy] [value]
//97-100 are joystick values
//101-108 are button values
int current_vals[12];

//pin the h-bridge/solenoid is on
int solenoid_pin = 3;

long int old_time = 0;

int axis = 0;
//Distinguishes from button / joystick commands
void check_command()
{
  //current_vals[4] = 0; //reset solenoid status
  
  //incoming_command[0] is the encoded value being converted to decimal
  if(incoming_command[0] >= 97 && incoming_command[0] <= 100) //axis a-d
  {
    axis = incoming_command[0];
    current_vals[(axis - 97)] = map(int(incoming_command[1]), 0, 200, -127, 127);
    /*Serial.print("Joystick ");
    Serial.print(incoming_command[0], HEX);
    Serial.print(" ");
    Serial.print(map(incoming_command[1], 0, 200, -127, 127));
    Serial.println();*/
  }
  else if (incoming_command[0] == 'G') //button 3 on controller
  {
    current_vals[4] = 1; //fire solenoid
  }

  Serial.println(current_vals[4]);
}

int drive;
long right_drive;
long left_drive;

//houses the code for driving the robot
void drive_bot()
{
    if (current_vals[4] == 1)
    {
      old_time = millis();
      analogWrite(solenoid_pin, 200); //fire solenoid
    }
    /*else
    {
      analogWrite(solenoid_pin, 0); //retract solenoid
    }*/
    //right_drive = current_vals[0] * current_vals[0];
    //left_drive = current_vals[1] * current_vals[1];
    /*
    //current_vals[1] -= 30;
    if((current_vals[0] < 30 && current_vals[0] > 0) || (current_vals[0] > -20 && current_vals[0] <0))
    {
      //right_drive *= -1;
      current_vals[0] = current_vals[0] / 2;
    }
    if((current_vals[1] < 30 && current_vals[1] > 0  )|| (current_vals[1] > -20 && current_vals[1] <0))    {
      //right_drive *= -1;
      current_vals[1] = current_vals[1] / 2;
    }
    */
    //current_vals[0] = right_drive / 120;
    //current_vals[1] = left_drive / 120;

    //current_vals[0] = constrain(current_vals[0], -127, 127);
    //current_vals[1] = constrain(current_vals[1], -127, 127);
    

    Serial.println(current_vals[0]);
    Serial.println(current_vals[1]);
    Serial.println();
     if(current_vals[1] > 7) //left motors
     {
      BackMotors.BackwardM1(address, current_vals[1]);
      BackMotors.BackwardM2(address, current_vals[1]);
     }
     else if(current_vals[1] < -7) //left motors
     {
      BackMotors.ForwardM1(address, abs(current_vals[1]));
      BackMotors.ForwardM2(address, abs(current_vals[1]));
     }
    else 
    {
      BackMotors.ForwardM1(address, 0);
      BackMotors.ForwardM2(address, 0);
    }
    
    if(current_vals[0] > 7) //right motors
    {
      FrontMotors.BackwardM1(address, current_vals[0]);
      FrontMotors.ForwardM2(address, current_vals[0]);
    }
    else if(current_vals[0] < -7)
    {
      FrontMotors.ForwardM1(address, abs(current_vals[0]));
      FrontMotors.BackwardM2(address, abs(current_vals[0]));
    }
    else
    {
      FrontMotors.ForwardM1(address, 0);
      FrontMotors.BackwardM2(address, 0);
    }
}

void setup() {
  pinMode(3, OUTPUT); //pwm pin for h-bridge/solenoid
  
  Serial1.begin(9600);
  Serial.begin(9600);

  //init comms with motor controllers
  FrontMotors.begin(38400);
  BackMotors.begin(38400);

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
      //Serial.print("RECEIVED ");
      //Serial.print(incoming_command[0]);
      //Serial.println();
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
      //Serial.print(incomingByte, HEX);
      //Serial.print(" ");
    }
    //Serial.println(incomingByte, HEX);
  }

}

