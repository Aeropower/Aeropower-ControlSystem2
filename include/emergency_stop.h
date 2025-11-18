// EmergencyStop.h
#pragma once
#include <ESP32Servo.h>

#include "state.h"

class EmergencyStop : public State {
 public:
  explicit EmergencyStop(Servo& blades)
      : State("Emergency"), blades(blades) {}
  void onEnter() override;
  void onExit() override {}
  void handle() override {}
  void reset() override {}

 private:
  Servo& blades;
};
