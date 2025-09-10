#ifndef INCLUDE_PITCH_CONTROL_STATE_H_
#define INCLUDE_PITCH_CONTROL_STATE_H_
#include <ESP32Servo.h>

#include "State.h"
#include "gpio.h"
class PitchControlState : public State {
 private:
  Servo& blades;
  int targetRpm = 0.0;
  float Kp, Ki, Kd;

  float prevErr = 0.0;
  float integral = 0.0;
  unsigned long lastMs = 0;

  int outMin, outMax;

 public:
  explicit PitchControlState(Servo& bladesRef, float kp = 0.5f,
                             float ki = 0.01f, float kd = 0.0f, int outMin = 0,
                             int outMax = 180)
      : blades(bladesRef),
        Kp(kp),
        Ki(ki),
        Kd(kd),
        outMin(outMin),
        outMax(outMax) {}

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
