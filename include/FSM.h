#pragma once
#include <ESP32Servo.h>

#include "EmergencyStop.h"
#include "PitchControlState.h"
#include "Stall.h"
#include "TorqueControl.h"
#include "gpio.h"
#include "state.h"
/*This FSM machine changes the turbine state based on the wind speed read from
  the Modern Devices Rev. C anemometer*/
class FSM {
 protected:
  State* currentState = nullptr;

  PitchControlState pitchControlState;
  Stall stallState;
  TorqueControl torqueControlState;
  EmergencyStop emergencyStopState;

 private:
  float windSpeed = 0.0;

 public:
  // Constructor that receives a Servo reference
  explicit FSM(Servo& bladesServo);

  void handle();

  void reset();

  void updateWindSpeed(float speed) { windSpeed = speed; }
};
