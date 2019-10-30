//#include "RoboClaw.h"
#include "L289N.h"

#define address 0x80

int solenoid_pin = 44; //pin the h-bridge/solenoid is on
const int JOY_THRESH = 15;  //dead zone for the joystick

//RoboClaw FrontMotors(&Serial2, 10000); //right motors
//RoboClaw BackMotors(&Serial3, 10000);

//Set up motor controllers
L289N frontLeftMotor(2, 3, 4, true); //setup a motor object with pins 2 and 3 controlling direction and 4 controlling speed
L289N backLeftMotor(5, 6, 7, true);
L289N frontRightMotor(8, 9, 10);
L289N backRightMotor(11, 12, 13);


//Recieving byte
byte incomingByte = 0;

//Storage for bytes for decoding
byte incoming_command[2];
int queue_len = 0;

//storage for values being sent [button / joy] [value]
//97-100 are joystick values
//101-108 are button values
int current_vals[12];

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

  //Serial.println(current_vals[4]);
}

//houses the code for driving the robot
void drive_bot()
{
  //kicker
    if (current_vals[4] == 1)
    {
      old_time = millis();
      analogWrite(solenoid_pin, 200); //fire solenoid
    }
    
    //Print Joystick vals
    Serial.println(current_vals[0]);
    Serial.println(current_vals[1]);
    Serial.println();

    //Drive motors
    if(abs(current_vals[1]) > JOY_THRESH) //left motors
    {
      frontLeftMotor.setSpeedDirection(current_vals[1] * 2);
      backLeftMotor.setSpeedDirection(current_vals[1] * 2);
     }
    else 
    {
      frontLeftMotor.setSpeedDirection(0);
      backLeftMotor.setSpeedDirection(0);
    }
    
    if(abs(current_vals[0]) > JOY_THRESH) //right motors
    {
      frontRightMotor.setSpeedDirection(current_vals[0] * 2);
      backRightMotor.setSpeedDirection(current_vals[0] * 2);
    }
    else
    {
      frontRightMotor.setSpeedDirection(0);
      backRightMotor.setSpeedDirection(0);
    }
}

void setup() {
  pinMode(solenoid_pin, OUTPUT); //pwm pin for h-bridge/solenoid
  
  Serial1.begin(9600);
  Serial.begin(9600);

  //initialize the motorsset
  frontLeftMotor.init();
  backLeftMotor.init();
  frontRightMotor.init();
  frontLeftMotor.init();

  //init comms with motor controllers
  //FrontMotors.begin(38400);
  //BackMotors.begin(38400);

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
