// TorqueControl.h
#pragma once
#include <ESP32Servo.h>

#include "state.h"

class TorqueControl : public State {
 public:
  explicit TorqueControl(Servo& servo) : State("TorqueControl"), servo(servo) {}
  void onEnter() override;
  void onExit() override;
  void handle() override;
  void reset() override;

 private:
  Servo& servo;
};

// Look up table for Duty Cycle based on RPM
typedef struct {
  float rpm;        // Revolutions per minute
  float dutyCycle;  // Duty cycle percentage
} DutyCycle_t;

// Example LUT, we have to substitute with real values after tuning
static const DutyCycle_t DUTY_LUT[] = {
    {0.0, 0.80f},   // 0
    {5.0, 1.60f},   // 5
    {10.0, 233.0f},  // 10
    {15.0, 512.0f},  // 15
    {18.0, 1023.0f}   // 18
};

// Linear interpolation helper
static float interpolateDutyCycle(float x0, float x1, float y0, float y1, float x) {
  if (x1 - x0 == 0) return y0;  // Prevent division by zero
  return y0 + (y1 - y0) * ((x - x0) / (x1 - x0));
}

// Function to get duty cycle from RPM using LUT
void getDutyCycle(float rpm, const DutyCycle_t* lut, int size, float* dutyCycle_out);

static const uint8_t DUTY_CYCLE_LUT_LEN =
    sizeof(DUTY_LUT) / sizeof(DutyCycle_t);
             
