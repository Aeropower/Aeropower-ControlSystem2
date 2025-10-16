// Stall.h
#pragma once
#include <ESP32Servo.h>

#include "State.h"

class Stall : public State {
 public:
  explicit Stall(Servo& blades) : State("Stall"), blades(blades) {}
  void onEnter() override {}
  void onExit() override {}
  void handle() override {}
  void reset() override {}
  private:
  Servo& blades;
};
