// Stall.h
#pragma once
#include <ESP32Servo.h>

#include "State.h"

class Stall : public State {
 public:
  explicit Stall(Servo&) {}   // Constructor vacío
  void onEnter() override {}  // Métodos vacíos
  void onExit() override {}
  void handle() override {}
  void reset() override {}
};
