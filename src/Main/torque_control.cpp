#include "torque_control.h"

#include <Arduino.h>  // For millis(), constrain(), etc.
#define TORQUE_CONTROL_ANGLE_DEGREES 45
#include "common.h"
#include "gpio.h"
#include "telemetry.h"

// Called when entering state
void TorqueControl::onEnter() {
  Serial.println("Entering TorqueControl State...");
  Telemetry t{};
  t.state = this->getName();
  telemetry_set_state(t.state);
  int currentAngle = servo.read();
  moveServoSmooth(servo, currentAngle, TORQUE_CONTROL_ANGLE_DEGREES, 1, 10);
  // Set an initial blade angle for torque control
  t.blade_angle = TORQUE_CONTROL_ANGLE_DEGREES;
  telemetry_set_blade_angle(servo.read());
}

// Update the state logic
void TorqueControl::handle() {
  // Read RPM from telemetry
  Telemetry t{};
  telemetry_get_snapshot(t);
  const float rpm = t.rpm;

  // Get Duty Cycle from LUT
  float dutyCycle = 0.0;
  getDutyCycle(rpm, DUTY_LUT, DUTY_CYCLE_LUT_LEN, &dutyCycle);

  // Apply Duty Cycle to PWM to control SEPIC converter
  ledcWrite(pwmChannel, static_cast<uint32_t>(dutyCycle));
  Serial.printf("TorqueControl State: RPM=%.2f, DutyCycle=%.2f\n", rpm,
                dutyCycle);
}

// Called when exiting state
void TorqueControl::onExit() {
  Serial.println("Exiting TorqueControl State...");
}

// Function to get duty cycle from RPM using LUT
void getDutyCycle(float rpm, const DutyCycle_t* lut, int size,
                  float* dutyCycle_out) {
  // Clamp if outside the table range
  if (rpm <= lut[0].rpm) {
    *dutyCycle_out = lut[0].dutyCycle;
    return;
  }
  if (rpm >= lut[size - 1].rpm) {
    *dutyCycle_out = lut[size - 1].dutyCycle;
    return;
  }

  // Find the interval
  for (int i = 0; i < size - 1; i++) {
    float x0 = lut[i].rpm;
    float x1 = lut[i + 1].rpm;
    if (rpm == x0) {
      *dutyCycle_out = lut[i].dutyCycle;
      return;
    }
    if (rpm >= x0 && rpm <= x1) {
      float y0 = lut[i].dutyCycle;
      float y1 = lut[i + 1].dutyCycle;
      *dutyCycle_out = interpolateDutyCycle(x0, x1, y0, y1, rpm);
      return;
    }
  }

  // Should not reach here
  *dutyCycle_out = lut[0].dutyCycle;
}

// Reset state variables
void TorqueControl::reset() {
  // Nothing to reset for now
}
