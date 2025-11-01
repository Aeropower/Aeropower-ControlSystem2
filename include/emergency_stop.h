// EmergencyStop.h
#pragma once
#include "state.h"
#include <ESP32Servo.h>

class EmergencyStop : public State {

public:
  explicit EmergencyStop(Servo& blades) : State("EmergencyStop"), blades(blades) {}
  void onEnter() override {}
  void onExit() override {}
  void handle() override {}
  void reset() override {}
  private:
  Servo& blades;
};
