#include "pitch_control.h"

#include <Arduino.h>  // For millis(), constrain(), etc.

#include "common.h"
#include "telemetry.h"

// Interpolation
void getPIDGains(float wind, const PID_Gains_t* lut, int size, float* Kp_out,
                 float* Ki_out) {
  // Clamp if outside the table range
  if (wind <= lut[0].wind_speed_mps) {
    *Kp_out = lut[0].Kp;
    *Ki_out = lut[0].Ki;
    return;
  }
  if (wind >= lut[size - 1].wind_speed_mps) {
    *Kp_out = lut[size - 1].Kp;
    *Ki_out = lut[size - 1].Ki;
    return;
  }

  // Find the interval
  for (int i = 0; i < size - 1; i++) {
    float x0 = lut[i].wind_speed_mps;
    float x1 = lut[i + 1].wind_speed_mps;

    if (wind == x0) {
      *Kp_out = lut[i].Kp;
      *Ki_out = lut[i].Ki;
      return;
    }

    if (wind >= x0 && wind <= x1) {
      float y0_kp = lut[i].Kp;
      float y1_kp = lut[i + 1].Kp;
      float y0_ki = lut[i].Ki;
      float y1_ki = lut[i + 1].Ki;

      *Kp_out = interpolatePID(x0, x1, y0_kp, y1_kp, wind);
      *Ki_out = interpolatePID(x0, x1, y0_ki, y1_ki, wind);
      return;
    }
  }
  // Should not reach here
  *Kp_out = lut[0].Kp;
  *Ki_out = lut[0].Ki;
}

// Called when entering the state
void PitchControlState::onEnter() {
  Serial.println("Entering PitchControlState...");
  Telemetry t{};
  t.state = this->getName();
  telemetry_set_state(t.state);
  integral = 0.0;
  prevErr = 0.0;
  lastMs = millis();
}


// Called when exiting the state
void PitchControlState::onExit() {
  Serial.println("Exiting PitchControlState...");
  // Let the duty cycle of the sepic converter to be the duty cycle at the rated
  // RPM's
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
  // float derivative = (error - prevErr) / dt;
  getPIDGains(t.wind_mps, PID_GAIN_LUT, PID_GAIN_LUT_LEN, &Kp, &Ki);
  // float output = Kp * error + Ki * integral + Kd * derivative;
  float output = Kp * error + Ki * integral;
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
  int currentAngle = blades.read();
  moveServoSmooth(blades, currentAngle, angle, 1, 10);
}

// Reset state internals
void PitchControlState::reset() {
  integral = 0.0;
  prevErr = 0.0;
  lastMs = millis();
}
