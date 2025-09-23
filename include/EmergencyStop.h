// EmergencyStop.h
#pragma once
#include "State.h"
#include <ESP32Servo.h>

class EmergencyStop : public State {
public:
  explicit EmergencyStop(Servo&) {}
  void onEnter() override {}
  void onExit() override {}
  void handle() override {}
  void reset() override {}
};
