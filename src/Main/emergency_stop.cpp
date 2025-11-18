#include "emergency_stop.h"

#include "common.h"
#include "gpio.h"
#include "telemetry.h"

// Called when entering the state
void EmergencyStop::onEnter() {
  Serial.println("Entering Emergency Stop State...");
  Telemetry t{};
  t.state = this->getName();
  telemetry_set_state(t.state);
  // Move blades to emergency stop angle
  int currentAngle = blades.read();
  moveServoSmooth(blades, currentAngle, STALL_ANGLE_DEGREES, 1, 10);

  // Set PWM to a duty cycle where the SEPIC converter forces generator to brake
  ledcWrite(pwmChannel, 1023);  // Max duty cycle to force braking

  // Activate necessary relays or mechanisms to stop the turbine safely and to
  // disconnect MCU from the turbine generator to the capacitor bank
}
