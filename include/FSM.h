#pragma once
#include <ESP32Servo.h>

#include "emergency_stop.h"
#include "pitch_control.h"
#include "stall.h"
#include "torque_control.h"
#include "gpio.h"
#include "state.h"
/*This FSM machine changes the turbine state based on the RPM readed from the Hall sensor*/
class FSM {
 protected:
  State* currentState = nullptr;

  PitchControlState pitchControlState;
  Stall stallState;
  TorqueControl torqueControlState;
  EmergencyStop emergencyStopState;


 public:
  // Constructor that receives a Servo reference
  explicit FSM(Servo& bladesServo);

  void handle();

  void reset();

  void initFSM();
};
