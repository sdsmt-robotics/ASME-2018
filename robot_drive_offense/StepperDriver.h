/* 1/10/2020
 * Samuel Ryckman
 * 
 * Header for the Stepper driver class.
 */

#ifndef STEPPER_DRIVER_H
#define STEPPER_DRIVER_H

#include "Arduino.h"

class StepperDriver {
public:
    StepperDriver(int stepPin, int dirPin, int enPin);
  
    void init();
    void enable();
    void disable();
    void stepForward();
    void stepBackward();
  
private:
    void step();
    
    int stepPin, dirPin, enPin;
    bool dirForward = true;  //current direction for the driver
};

#endif
