//#include "RoboClaw.h"
#include "Controller.h"
#include "L289N.h"

#define KICK_LEN 250 // Length of a kick in ms

int solenoid_pin = 44; //pin the h-bridge/solenoid is on

//Create the communications object. Use Serial for the communications.
Controller controller(Serial2);

//Set up motor controllers
L289N frontLeftMotor(2, 3, 4, true); //setup a motor object with pins 2 and 3 controlling direction and 4 controlling speed
L289N backLeftMotor(5, 6, 7, true);
L289N frontRightMotor(8, 9, 10);
L289N backRightMotor(11, 12, 13);

void setup() {
  Serial.begin(115200);
  
  //initialize the receiver
  controller.init();
  Serial.println("Waiting for connection...");
  while (!controller.connected()) { delay(10); }
  Serial.println("Connected!");

  //set a deadzone for the joysticks
  controller.setJoyDeadzone(0.08);

  Serial.println("Initializing motors...");
  //initialize the motors
  frontLeftMotor.init();
  backLeftMotor.init();
  frontRightMotor.init();
  frontLeftMotor.init();

  // Solenoid init
  pinMode(solenoid_pin, OUTPUT); //pwm pin for h-bridge/solenoid
  // Do some kiks
  analogWrite(solenoid_pin, 200);
  delay(500);
  analogWrite(solenoid_pin, 0);
  delay(500);
  analogWrite(solenoid_pin, 200);
  delay(500);
  analogWrite(solenoid_pin, 0);
  Serial.println("Done!");

  Serial.println("Starting main loop.");
}

void loop() {
  static bool disconnected = false;
  
  if (controller.connected()) {
    if (disconnected) {
      Serial.println("Reconnected!");
      disconnected = false;
    }
    
    // Kicker
    if (controller.button(DOWN)) {
      analogWrite(solenoid_pin, 200); // fire solenoid
    } else {
      analogWrite(solenoid_pin, 0); // Retract solenoid
    }
    
    // Driving
    int leftSpeed = controller.joystick(LEFT, Y) * 255;
    int rightSpeed = controller.joystick(RIGHT, Y) * 255;
    frontLeftMotor.setSpeedDirection(leftSpeed);
    backLeftMotor.setSpeedDirection(leftSpeed);
    frontRightMotor.setSpeedDirection(rightSpeed);
    backRightMotor.setSpeedDirection(rightSpeed);
    
  } else {
    if (!disconnected) {
      Serial.println("Disconnected! Waiting for reconnect...");
      disconnected = true;
      
      analogWrite(solenoid_pin, 0); // Retract solenoid
      frontLeftMotor.setSpeedDirection(0);
      backLeftMotor.setSpeedDirection(0);
      frontRightMotor.setSpeedDirection(0);
      backRightMotor.setSpeedDirection(0);
    }
  }
}
