// TorqueControl.h
#pragma once
#include "State.h"
#include <ESP32Servo.h>

class TorqueControl : public State {
public:
  explicit TorqueControl(Servo& servo) : State("TorqueControl"), servo(servo) {}
  void onEnter() override {}
  void onExit() override {}
  void handle() override {}
  void reset() override {}
private:
  Servo& servo;
};
