#ifndef INCLUDE_PITCH_CONTROL_STATE_H_
#define INCLUDE_PITCH_CONTROL_STATE_H_
#include <ESP32Servo.h>
#define RATED_SPEED_RPM 2000
#include <string>

#include "gpio.h"
#include "state.h"

// Look up table for PID
typedef struct {
  float wind_speed_mps;  // Wind speed in meters per second
  float Kp;              // Kp in float format
  float Ki;              // Ki in float format
  // float Kd;      //Kd in float format maybe we dont need derivative term
} PID_Gains_t;

// Example LUT, we have to substitute with real values after tuning
static const PID_Gains_t PID_GAIN_LUT[] = {
    {0.0, 0.80f, 0.15f},   // 0 m/s
    {5.0, 1.20f, 0.25f},   // 5 m/s
    {10.0, 1.60f, 0.30f},  // 10 m/s
    {15.0, 2.00f, 0.35f},  // 15 m/s
    {18.0, 2.40f, 0.40f}   // 18 m/s
};

// Linear interpolation helper
static float interpolatePID(float x0, float x1, float y0, float y1, float x) {
  if (x1 - x0 == 0) return y0;  // Prevent division by zero
  return y0 + (y1 - y0) * ((x - x0) / (x1 - x0));
}

void getPIDGains(float wind, const PID_Gains_t* lut, int size, float* Kp_out,
                 float* Ki_out);
static const uint8_t PID_GAIN_LUT_LEN =
    sizeof(PID_GAIN_LUT) / sizeof(PID_Gains_t);

class PitchControlState : public State {
 private:
  Servo& blades;
  int targetRpm = RATED_SPEED_RPM;
  float Kp, Ki, Kd;

  float prevErr = 0.0;
  float integral = 0.0;
  unsigned long lastMs = 0;
  const int outMin = 0;   // Minimum blade angle example
  const int outMax = 90;  // Maximum blade angle example

 public:
  explicit PitchControlState(Servo& bladesRef)
      : State("Pitch"), blades(bladesRef) {}

  ~PitchControlState() override = default;
  // Called when entering the state
  void onEnter() override;

  // Called when exiting the state
  void onExit() override;

  // Main action of the state
  void handle() override;

  // Everytime we reset everything
  void reset() override;

  void setTargetRPM(float rpm) noexcept { targetRpm = rpm; }
  float getTargetRPM() const noexcept { return targetRpm; }
};

#endif
