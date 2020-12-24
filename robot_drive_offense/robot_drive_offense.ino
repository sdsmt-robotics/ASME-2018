#include "RoboClaw.h"
#include "Controller.h"
#include <SoftwareSerial.h>
#include <PololuMaestro.h>

#define address 0x80

#define stp 22
#define dir 24
#define MS1 26
#define MS2 28
#define EN  30

//Drive motors
/* Front:
    M1 - left
    M2 - right

    Back:
     M1 - left
     M2 - right
*/

RoboClaw FrontMotors(&Serial2, 100);
RoboClaw BackMotors(&Serial3, 100);

SoftwareSerial smcSerial1 = SoftwareSerial(4, 5); //right
SoftwareSerial smcSerial2 = SoftwareSerial(7, 6); ///left

//XBee receiver setup
Controller controller(Serial1);
//sam thinks this is stupid, sends serial data to Controller object when available
void serialEvent1()
{
  controller.receiveData();
}

//joystick differencing - have they moved?
int joyLX_last = 0;
int joyLY_last = 0;
int joyRX_last = 0;

//time differencing - sending too much?
long long unsigned int prevTime = 0;

//Servo controller
//SoftwareSerial(rxPin, txPin)
SoftwareSerial maestroSerial(10, 11);
MicroMaestro maestro(maestroSerial);

//Recieving byte
byte incomingByte = 0;
bool shooting = false;

//Storage for bytes for decoding
byte incoming_command[2];
int queue_len = 0;

//storage for values being sent [button / joy] [value]
//97-100 are joystick values
//101-108 are button values
int current_vals[12];

int axis = 0;

bool stage2 = true;
bool stage3 = false;

bool stepper = false;
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
  if (incoming_command[0] <= 100 && incoming_command[0] >= 97)
  {
    axis = incoming_command[0];
    current_vals[(axis - 97)] = map(int(incoming_command[1]), 0, 200, -127, 127);
    /*Serial.print("Joystick ");
      Serial.print(incoming_command[0], HEX);
      Serial.print(" ");
      Serial.print(map(incoming_command[1], 0, 200, -127, 127));
      Serial.println();*/
  }
  else if (incoming_command[0] == 69)
  {
    Serial.println("Stopping Shooter");
    setMotorSpeed(0);
    shooting = false;
  }
  else if (incoming_command[0] == 70)
  {
    Serial.println("Starting Shooter");
    //max
    setMotorSpeed(700);
    delay(20);
    setMotorSpeed(510);
    shooting = true;
  }
  else if (incoming_command[0] == 71)
  {
    //low
    Serial.println("Starting Shooter");
    setMotorSpeed(700);
    delay(20);
    setMotorSpeed(350);
    shooting = true;
  }
  else if (incoming_command[0] == 72)
  {
    //mid
    Serial.println("Starting Shooter");
    setMotorSpeed(700);
    delay(20);
    setMotorSpeed(420 - 10);
    shooting = true;
  }
  else if (incoming_command[0] == 73) //right side, left button
  {
    Serial.println("Toggle stepper");
    if (!stepper)
    {
      //forward
      digitalWrite(EN, LOW); //Pull enable pin low to allow motor control
      stepper = true;
    }
    else if (stepper)
    {
      stepper = false;
      resetEDPins();
    }
    else
    {
      stepper = false;
      resetEDPins();
    }
  }
  else if (incoming_command[0] == 74)
  {
    Serial.println("Starting Shooter Max Speed");

    for (int i = 1; i < 32; i++)
    {
      setMotorSpeed(100 * i);
      Serial.println(100 * i);
      delay(200);
    }
    setMotorSpeed(3200);
    shooting = true;
  }
  else if (incoming_command[0] == 75)  //right side bottom button
  {
    if (stage2)
    {
      //medium
      maestro.setTarget(0, 6000);
      maestro.setTarget(1, 5960);
      stage2 = false;
      stage3 = true;
    }
    else if (stage3)
    {
      //closed
      maestro.setTarget(0, 8100);
      maestro.setTarget(1, 3600);
      stage3 = false;
      stage2 = true;
    }
    else
    {
      //fail-back to open
      maestro.setTarget(0, 9800);
      maestro.setTarget(1, 3968);
      stage2 = false;
      stage3 = false;
    }
  }
  else if (incoming_command[0] == 76)
  {
    //ehhh
    Serial.println("Starting Shooter");
    setMotorSpeed(700);
    delay(20);
    setMotorSpeed(300);
    shooting = true;
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
  /*if(stepper)
    {
    current_vals[1] = 0;
    current_vals[3] = 0;
    current_vals[2] = current_vals[2] / 2;
    }*/

  //current_vals[1] ---> left y  (pos=up)
  //current_vals[2] ---> left x  (pos=left)
  //current_vals[3] ---> right x  (pos=left)
  front_right = current_vals[1] - current_vals[2] + current_vals[3];
  rear_right  = -current_vals[1] - current_vals[2] - current_vals[3];
  front_left  = current_vals[1] + current_vals[2] - current_vals[3];
  rear_left   = -current_vals[1] + current_vals[2] + current_vals[3];

  front_right = constrain(front_right, -127, 127);
  rear_right  = constrain(rear_right, -127, 127);
  front_left  = constrain(front_left, -127, 127);
  rear_left   = constrain(rear_left, -127, 127);


  // Determines direction and speed of each motor
  if (front_left > 5)
  {
    FrontMotors.ForwardM1(address, front_left);
  }
  else if (front_left < 5)
  {
    FrontMotors.BackwardM1(address, abs(front_left));
  }
  else
  {
    FrontMotors.ForwardM2(address, 0);
  }

  if (front_right > 5)
  {
    FrontMotors.BackwardM2(address, front_right);
  }
  else if (front_right < 5)
  {
    FrontMotors.ForwardM2(address, abs(front_right));
  }
  else
  {
    FrontMotors.ForwardM2(address, 0);
  }

  if (rear_left > 5)
  {
    BackMotors.ForwardM1(address, rear_left);
  }
  else if (rear_left < 5)
  {
    BackMotors.BackwardM1(address, abs(rear_left));
  }
  else
  {
    BackMotors.ForwardM1(address, 0);
  }

  if (rear_right > 5)
  {
    BackMotors.BackwardM2(address, rear_right);
  }
  else if (rear_right < 5)
  {
    BackMotors.ForwardM2(address, abs(rear_right));
  }
  else
  {
    BackMotors.ForwardM2(address, 0);
  }
}

void setup() {
  pinMode(stp, OUTPUT);
  pinMode(dir, OUTPUT);
  pinMode(MS1, OUTPUT);
  pinMode(MS2, OUTPUT);
  pinMode(EN, OUTPUT);
  resetEDPins(); //Set step, direction, microstep and enable pins to default states

  Serial.begin(115200);

  //initialize the receiver
  controller.init();
  Serial.println("Waiting for connection...");
  while (!controller.connected()) {
    delay(10);
  }
  Serial.println("Connected...");
  //set a deadzone for the joysticks
  controller.setJoyDeadzone(0.08);

  //init comms with motor controllers
  FrontMotors.begin(38400);
  BackMotors.begin(38400);

  // initialize software serial object with baud rate of 19.2 kbps
  smcSerial1.begin(19200);
  smcSerial2.begin(19200);

  maestroSerial.begin(9600);

  delay(5);

  smcSerial1.write(0xAA);  // send baud-indicator byte
  smcSerial2.write(0xAA);  // send baud-indicator byte

  exitSafeStart();  // clear the safe-start violation and let the motor run

  maestro.setTarget(0, 3968);
  maestro.setTarget(1, 8000);
  Serial.println("Starting Recieve Code");
}

//Reset Easy Driver pins to default states
void resetEDPins()
{
  digitalWrite(stp, LOW);
  digitalWrite(dir, LOW);
  digitalWrite(MS1, LOW);
  digitalWrite(MS2, LOW);
  digitalWrite(EN, HIGH);
}

unsigned int beginloop, parsetime, drivebot;

void loop() {
  beginloop = millis();

  parseControllerInput();

  parsetime = millis();

  //if (joysticksHaveMoved() && waitedLongEnough())
  {
    drive_bot();
  }

  drivebot = millis();

  int loopTime = drivebot - parsetime;
  Serial.println(loopTime);

  //  if (Serial1.available() > 0)
  //  {
  //    incomingByte = Serial1.read();
  //    if (incomingByte == 0x58)
  //    {
  //      Serial.println();
  //      check_command();
  //      drive_bot();
  //      queue_len = 0;
  //    }
  //    else
  //    {
  //      if(queue_len >= 2)
  //      {
  //        queue_len = 0;
  //      }
  //      incoming_command[queue_len] = incomingByte;
  //      queue_len++;
  //      //Serial.print(incomingByte, HEX);
  //      //Serial.print(" ");
  //    }
  //    //Serial.println(incomingByte, HEX);
  //  }
  //  if(stepper)
  //  {
  //    digitalWrite(stp,HIGH); //Trigger one step forward
  //    delay(1);
  //    digitalWrite(stp,LOW); //Pull step pin low so it can be triggered again
  //    delay(1);
  //  }
}

void parseControllerInput()
{
  //current_vals[1] ---> left y  (pos=up)
  //current_vals[2] ---> left x  (pos=left)
  //current_vals[3] ---> right x  (pos=left)

  current_vals[1] = controller.joystick(LEFT, Y) * 255;
  current_vals[2] = controller.joystick(LEFT, X) * 255;
  current_vals[3] = controller.joystick(RIGHT, X) * 255;

  //Serial.println(current_vals[1]);
}

bool joysticksHaveMoved()
{
  if (
    abs(current_vals[1] - joyLY_last) > 5 ||
    abs(current_vals[2] - joyLX_last) > 5 ||
    abs(current_vals[3] - joyRX_last) > 5
  )
  {
    joyLY_last = current_vals[1];
    joyLX_last = current_vals[2];
    joyRX_last = current_vals[3];
    return true;
  }
  else
  {
    return false;
  }
}

bool waitedLongEnough()
{
  if (millis() - prevTime > 50)
  {
    prevTime = millis();
    return true;
  }
  else
  {
    return false;
  }
}
