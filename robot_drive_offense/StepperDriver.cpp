/* 1/10/2020
 * Samuel Ryckman
 */
 
#include "StepperDriver.h"

/** Constructor for the class.
*/
StepperDriver::StepperDriver(int stepPin, int dirPin, int enPin)
    : stepPin(stepPin), dirPin(dirPin), enPin(enPin)
{
}

/**
 * Initialize the output
 */
void StepperDriver::init() {
    pinMode(stepPin, OUTPUT);
    pinMode(dirPin, OUTPUT);
    pinMode(enPin, OUTPUT);

    digitalWrite(dirPin, LOW);
    digitalWrite(stepPin, LOW);
    disable();
}

/**
 * Enable the driver
 */
void StepperDriver::enable() {
  digitalWrite(enPin, LOW);
}

/**
 * Disable the driver
 */
void StepperDriver::disable() {
  digitalWrite(enPin, HIGH);
}

/**
 * Do a step in the current direction.
 */
void StepperDriver::step() {
    digitalWrite(stepPin, HIGH); //Trigger one step forward
    delay(1);
    digitalWrite(stepPin, LOW); //Pull step pin low so it can be triggered again
    delay(1);
}

/**
 * Set direction to forward and do a step.
 */
void StepperDriver::stepForward() {
    //Adjust direction if needed
    if (!dirForward) {
        digitalWrite(dirPin, LOW);
        dirForward = true;
    }
    step();
}

/**
 * Set direction to backward and do a step.
 */
void StepperDriver::stepBackward() {
    //Adjust direction if needed
    if (dirForward) {
        digitalWrite(dirPin, HIGH);
        dirForward = false;
    }
    step();
}