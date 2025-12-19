#pragma once
#include <ESP32Servo.h>

#include "emergency_stop.h"
#include "gpio.h"
#include "pitch_control.h"
#include "stall.h"
#include "state.h"
#include "torque_control.h"
/*This FSM machine changes the turbine state based on the RPM readed from the
 * Hall sensor*/
class FSM {
 protected:
  Servo& bladesServo;
  State* currentState = nullptr;
  hw_timer_t* timer = nullptr;
  static FSM* instance;

  PitchControlState pitchControlState;
  Stall stallState;
  TorqueControl torqueControlState;
  EmergencyStop emergencyStopState;
  volatile bool transition_ready = false;

 public:
  // Constructor that receives a Servo reference
  explicit FSM(Servo& bladesServo);

  void handle();

  void reset();

  void initFSM();

  static void IRAM_ATTR onTimerISR();
};
