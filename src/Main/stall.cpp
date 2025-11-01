#include "stall.h"

#include <Arduino.h>  // For millis(), constrain(), etc.

#include "common.h"
#include "gpio.h"

// Called when entering the state
void Stall::onEnter() {
  Serial.println("Entering Stall State...");
  // Move blades to stall angle
  int currentAngle = blades.read();
  moveServoSmooth(blades, currentAngle, STALL_ANGLE_DEGREES, 1, 10);
  esp_sleep_enable_ext0_wakeup((gpio_num_t)RIGHT_BUTTON,
                               1);  // Wake up on button press
  esp_sleep_enable_ext0_wakeup((gpio_num_t)LEFT_BUTTON,
                               1);   // Wake up on button press
  esp_sleep_enable_timer_wakeup(600e6);  // Wake up after 10 minutes
}

//Update the state logic
void Stall::handle() {
  Serial.println("Stall State: Entering light sleep...");
  esp_light_sleep_start();
  Serial.println("Stall State: Woke up from light sleep.");
  delayMicroseconds(50); //So the other cores can read sensor and see it the turbine is still on stall
}

// Called when exiting the state
void Stall::onExit() {
  Serial.println("Exiting Stall State...");
}

// Reset function (not used in this state)
void Stall::reset() {
  // No specific reset actions needed for Stall state
}
