#include "RoboClaw.h"
#include "Controller.h"
#include <SoftwareSerial.h>
#include <PololuMaestro.h>
#include "StepperDriver.h"
#include "MusicPlayer.h"

//Stepper motor
//StepperDriver(stepPin, dirPin, enPin);
StepperDriver stepper(22, 24, 26);

//Drive motors
RoboClaw leftMotors(&Serial2, 100);
RoboClaw rightMotors(&Serial3, 100);

//Shooty motors
#define LEFT_SHOOTY_PIN 6
#define RIGHT_SHOOTY_PIN 7
#define STOP_SUPER_FAST_SHOOTY_PIN 52
#define STOP_FAST_SHOOTY_PIN 53
#define NO_SHOOTY_RESTRICT 0            //Does not restrict any speed
#define SUPER_FAST_SHOOTY_RESTRICT 1    //Restricts "DEATH" speed only
#define FAST_SHOOTY_RESTRICT 2          //Restricts "HIGH" and "DEATH" speeds


//SoftwareSerial(rxPin, txPin)
//BTS7960(rPwmPin, lPwmPin, enPin, invert)
//BTS7960 leftShooter(4, 3, 7);
//SoftwareSerial rightShooterSerial = SoftwareSerial(7, 6); ///right
//SoftwareSerial leftShooterSerial = SoftwareSerial(4, 5); //left

//Servo controller
//SoftwareSerial(rxPin, txPin)
const int LEFT_SERVO = 0;
const int RIGHT_SERVO = 1;
SoftwareSerial servoDriverSerial(10, 11);
MicroMaestro servoDriver(servoDriverSerial);

//XBee receiver setup
Controller controller(Serial1);
//sam thinks this is stupid, sends serial data to Controller object when available
void serialEvent1() {
  controller.receiveData();
}

// Music player
//MusicPlayer(pwmPin6, pwmPin7, pwmPin8)
MusicPlayer musicPlayer(true, true, false);


//=====SETUP=============================================
void setup() {
  Serial.begin(115200);

  //initialize the receiver
  controller.init();
  Serial.println("Waiting for connection...");
  while (!controller.connected()) {
    delay(10);
  }
  Serial.println("Connected!");
  //set a deadzone for the joysticks
  controller.setJoyDeadzone(0.08);

  Serial.println("Initializing motor drivers...");
  // Drive motors
  leftMotors.begin(38400);
  rightMotors.begin(38400);

  //Shooter motors
  configPwm();
  pinMode(RIGHT_SHOOTY_PIN, OUTPUT);
  pinMode(LEFT_SHOOTY_PIN, OUTPUT);
  pinMode(STOP_SUPER_FAST_SHOOTY_PIN, INPUT_PULLUP);
  pinMode(STOP_FAST_SHOOTY_PIN, INPUT_PULLUP);

  // Servo motors
  servoDriverSerial.begin(9600);

  // Intake motor
  stepper.init();

  Serial.println("Done!");
  Serial.println("Starting main loop.");
}

//=====MAIN LOOP=============================================
void loop() {
  controlDriving();
  controlFlaps();
  controlScoop();
  controlShooter();
  controlMusic();
}

/**
 * Config the PWM on pins 6, 7, 8 to run at 25kHz so it is inaudible.
 * This reduces the usable range of values to 0-80.
 */
void configPwm() {
    /*
    A bit of explanation for those who care...
    On the Arduino Mega, there are 4 16-bit timer/counters (1, 3, 4, and 5). Timer 2 is 
    special and used by millis() and delay(), so don't mess with it. Each timer is used 
    by 2-3 pins.
    Pins 6, 7, 8 use timer 4.
    Description of registers (n = timer #, x = pin letter):
      - TCCRnA/B/C: Timer/Counter control registers A, B, and C. Set timer source, mode, etc.
      - OCRnx - Output compare register. Value set for the pin to control duty cycle.
      - ICRn - Input caputer register. Used as the TOP (reset) value in phase correct PWM mode.
      - TCNTn - Timer/Counter for the clock. The current count for the cycle.

    OCnx is the output.
    */

    // Clear the control registers.
    TCCR4A = 0;
    TCCR4B = 0;
    TCNT4  = 0;

    // Mode 10: phase correct PWM with ICR4 as Top (= F_CPU/2/25000)
    // OC4C as Non-Inverted PWM output
    // TOP = 511
    // Bits set:
    //  - COM = 0b10 (clear output on match when upcounting, set output on match when downcounting)
    //  - WGM = 0b1010 (wave generation mode = phase correct PWM with ICR4 as the prescaler)
    //  - CS = 0b001 (Clock source 1, no prescaling)
    ICR4   = (F_CPU/2000)/2;      // Get the prescaler for 25k. Have to divide by two since phase correct.
    OCR4A  = 0;                    // Set initial duty cycle to 0%
    OCR4B  = 0;                    // Set initial duty cycle to 0%
    OCR4C  = 0;                    // Set initial duty cycle to 0%
    TCCR4A = _BV(COM4A1) | _BV(COM4B1) | _BV(COM4C1) | _BV(WGM41);
    TCCR4B = _BV(WGM43) | _BV(CS40);

    Serial.println("Max duty cycle: "+String(ICR4));
}

/**
 * Control a single motor attached to a roboclaw motor driver.
 * 
 * @param motorDriver - the motor driver object
 * @param motorNum - the number of the motor to control. Either 1 or 2.
 * @param speed - power to set the motor to
 * @param invert - inver the direction
 */
void controlMotor(RoboClaw &motorDriver, int motorNum, int speed, bool invert = false) {
  const int deadzone = 5;
  const int address = 0x80; // Address for speed control command

  if (invert) {
    speed = -speed;
  }
  
  if (speed > deadzone) {
    if (motorNum == 2) {
      motorDriver.ForwardM2(address, speed);
    } else {
      motorDriver.ForwardM1(address, speed);
    }
  } else if (speed < -deadzone) {
    if (motorNum == 2) {
      motorDriver.BackwardM2(address, abs(speed));
    } else {
      motorDriver.BackwardM1(address, abs(speed));
    }
  } else {
    if (motorNum == 2) {
      motorDriver.ForwardM2(address, 0);
    } else {
      motorDriver.ForwardM1(address, 0);
    }
  }
}

/**
 * Control the drive motors based on joystick values.
 */
void controlDriving() {
  const int REAR_MOTOR = 1;
  const int FRONT_MOTOR = 2;
  static unsigned long lastUpdate = millis();

  if (millis() - lastUpdate > 10) {
    // Get the values from the controller if connected
    float leftY = 0;
    float leftX = 0;
    float rightX = 0;
    if (controller.connected()) {
      leftY = controller.joystick(LEFT, Y) * 255;
      leftX = controller.joystick(LEFT, X) * 255;
      rightX = controller.joystick(RIGHT, X) * 255;
    }
  
    // Calculate values for mecanum driving
    float frontRightPwr =  leftY + leftX + rightX;
    float rearRightPwr  = -leftY + leftX - rightX;
    float frontLeftPwr  =  leftY - leftX - rightX;
    float rearLeftPwr   = -leftY - leftX + rightX;
  
    // Constrain the values to within acceptable range
    frontRightPwr = constrain(frontRightPwr, -127, 127);
    rearRightPwr  = constrain(rearRightPwr, -127, 127);
    frontLeftPwr  = constrain(frontLeftPwr, -127, 127);
    rearLeftPwr   = constrain(rearLeftPwr, -127, 127);
    
    // Send the values to the motor controllers
    controlMotor(leftMotors, FRONT_MOTOR, frontLeftPwr);
    controlMotor(leftMotors, REAR_MOTOR, rearLeftPwr);
    controlMotor(rightMotors, FRONT_MOTOR, frontRightPwr, true);
    controlMotor(rightMotors, REAR_MOTOR, rearRightPwr, true);

    lastUpdate = millis();
  }
}

/**
 * Control the servos which run the front two flaps. DPAD buttons switch between 
 * three set positions. DPAD-down moves flaps toward robot, DPAD-up moves flaps away.
 */
void controlFlaps() {
  const int leftPositions[] = {8800, 8800-2000, 8800-4700};
  const int rightPositions[] = {3400, 3400+2000, 3400+4700};
  static int curState = 0;
  static int lastState = -1;

  // Get the current value from the controller
  if (controller.dpadClick(DOWN)) {
    curState--; // Move flaps in towards robot
  } else if (controller.dpadClick(UP)) {
    curState++; // Move flaps away from robot
  }
  curState = constrain(curState, 0, 2);

  // Set position if change state
  if (lastState != curState) {
    Serial.println("Setting flap position: "+String(curState));
    // Contol positions based on the state
    servoDriver.setTarget(LEFT_SERVO, leftPositions[curState]);
    servoDriver.setTarget(RIGHT_SERVO, rightPositions[curState]);
    
    lastState = curState;
  }
}

/**
 * Control the intake scoop. Running/stopped is toggled by DPAD-right.
 */
void controlScoop() {
  static bool scoopRunning = false;
  static unsigned long lastStep = millis();

  // Get a change of value from controller
  if (controller.dpadClick(RIGHT)) {
    scoopRunning = !scoopRunning;
    
    // Control the stepper
    if (scoopRunning) {
      Serial.println("Setting scoop state: RUNNING");
      stepper.enable();
    } else {
      Serial.println("Setting scoop state: STOPPED");
      stepper.disable();
    }
  }

  // Check to makes sure good to take another step (should be fine, but better safe than sorry)
  if (millis() - lastStep > 1) {
    stepper.stepForward();
    lastStep = millis();
  }

}

/**
 * Control the shooter. Set the appropriate speed based on button pressed.
 */
void controlShooter() {
  const int speeds[] = {0, ICR4*0.10, ICR4*0.20, ICR4*0.3, ICR4*0.6};
  static int curMode = 0;                     // current speed mode for the shooter
  static int lastMode = -1;                   // current speed mode for the shooter
  
  const unsigned long killModeTime = 3000;    //Amount of time button must be pressed to engage kill mode
  static unsigned long pressStartTime = millis(); // Time of the start of the high-speed button press
  static bool pressStarted = false;

  //Defines modes to restrict "HIGH" and "DEATH" speeds depending on the setting of a switch
  int shooterRestrictMode = 0;
  int pinA = digitalRead(STOP_SUPER_FAST_SHOOTY_PIN);
  int pinB = digitalRead(STOP_FAST_SHOOTY_PIN);
  if(!pinA) {  //restricts only "DEATH" speed(kill mode)
    shooterRestrictMode = SUPER_FAST_SHOOTY_RESTRICT;
  } else if (!pinB) { // restricts both "HIGH" and "DEATH" speeds
    shooterRestrictMode = FAST_SHOOTY_RESTRICT;
  } else { //no restrictions present
    shooterRestrictMode = NO_SHOOTY_RESTRICT;
  }
  // Handle button presses
  if (controller.buttonClick(DOWN)) {
    Serial.println("Setting shooter speed: OFF");
    curMode = 0;
  } else if (controller.buttonClick(LEFT)) {
    Serial.println("Setting shooter speed: LOW");
    curMode = 1;
  } else if (controller.buttonClick(UP)) {
    Serial.println("Setting shooter speed: MEDIUM");
      curMode = 2;
  } else if (controller.buttonClick(RIGHT) && shooterRestrictMode < 2) {
    if (curMode == 3 && !pressStarted) {// Start of a press to go into kill mode
      pressStartTime = millis();
      pressStarted = true;
    } else {
      Serial.println("Setting shooter speed: HIGH");
      curMode = 3;
      pressStarted = false;
    }
    setShooterSpeed(speeds[3]); //high
  } else if (!controller.button(RIGHT) && pressStarted && millis() - pressStartTime > killModeTime && curMode == 3 && shooterRestrictMode == 0) { // End of a press to go into kill mode
    Serial.println("Setting shooter speed: DEATH");
    curMode = 4;
    pressStarted = false;
  }

  // Off if disconnect
  if (!controller.connected() && curMode != 0) {
    Serial.println("Disconnect, setting shooter speed: OFF");
    curMode = 0;
  }

  // Send the speed to the controller if mode change
  if (lastMode != curMode) {
    setShooterSpeed(speeds[curMode]);
    lastMode = curMode;
  }
}

/**
 * Set the speed for the shooter motors.
 * @param speed - a number from -3200 to 3200
 */
void setShooterSpeed(int speed) {
  //Set the pwm duty cycles
  OCR4A = speed;
  OCR4B = speed;
  OCR4C = speed;
}

/**
 * Control the music. Play a song if the correct button is pressed.
 */
void controlMusic() {
  if (controller.dpadClick(LEFT)) {
    if (!musicPlayer.isPlaying()) {
      musicPlayer.playMerryChristmas(0.005);
    } else {
      musicPlayer.stop();
    }
  }

  musicPlayer.update();
}
