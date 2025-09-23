#include "PitchControlState.h"
#include <Arduino.h>  // For millis(), constrain(), etc.
#include "telemetry.h"

// Called when entering the state
void PitchControlState::onEnter() {
  Serial.println("Entering PitchControlState...");
  integral = 0.0;
  prevErr = 0.0;
  lastMs = millis();
}

// Called when exiting the state
void PitchControlState::onExit() {
  Serial.println("Exiting PitchControlState...");
}

// Main action of the state (called repeatedly in FSM)
void PitchControlState::handle() {

  Telemetry t{};
  telemetry_get_snapshot(t);

  unsigned long now = millis();
  float dt = (now - lastMs) / 1000.0f;  // convert ms to seconds
  lastMs = now;
  float rpm = t.rpm;
  float error = targetRpm - rpm;

  integral += error * dt;
  float derivative = (error - prevErr) / dt;

  float output = Kp * error + Ki * integral + Kd * derivative;
  prevErr = error;

  int angle = static_cast<int>(constrain(output, outMin, outMax));
  telemetry_set_blade_angle(angle);

  Serial.print("Target RPM: ");
  Serial.print(targetRpm);
  Serial.print(" | Current RPM: ");
  Serial.print(rpm);
  Serial.print(" | Servo Angle: ");
  Serial.println(angle);

  // Write to servo or actuator
  blades.write(angle);
}

// Reset state internals
void PitchControlState::reset() {
  integral = 0.0;
  prevErr = 0.0;
  lastMs = millis();
}
