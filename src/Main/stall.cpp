#include "stall.h"

#include <Arduino.h>  // For millis(), constrain(), etc.
#include <esp_sleep.h>
#include <time.h>

#include "common.h"
#include "driver/gpio.h"
#include "gpio.h"
#include "telemetry.h"

uint64_t sleepTime = 10000000;      // microseconds (10 seconds)
static bool firstSleep = true;      // track first sleep per state entry
static unsigned long lastWakeMs = 0;
static const unsigned long AWAKE_WINDOW_MS = 200;  // stay awake briefly after wake

// Called when entering the state
void Stall::onEnter() {
  // Turbine begins stalled, so it should be powered by a bank of capacitors or
  // a Li-ion battery to keep the MCU running while the turbine is not
  // generating power It is assumed that the components necessary for this are
  // already in place(relays, mosfets, etc.)
  Serial.println("Entering Stall State...");
  pinMode(LED_BUILTIN, OUTPUT);  // set built-in LED pin as output
  digitalWrite(LED_BUILTIN, LOW);
  // Move blades to stall angle
  delay(100);  // Small delay to ensure telemetry task is ready
  Telemetry t{};
  t.state = this->getName();
  telemetry_set_state(t.state);
  int currentAngle = blades.read();
  moveServoSmooth(blades, currentAngle, STALL_ANGLE_DEGREES, 1, 10);
  // Enable GPIO wakeup on the LCD buttons (active low)
  pinMode(RIGHT_BUTTON, INPUT_PULLUP);
  pinMode(LEFT_BUTTON, INPUT_PULLUP);
 // gpio_wakeup_enable((gpio_num_t)RIGHT_BUTTON, GPIO_INTR_LOW_LEVEL);
 // gpio_wakeup_enable((gpio_num_t)LEFT_BUTTON, GPIO_INTR_LOW_LEVEL);
 // esp_sleep_enable_gpio_wakeup();
}

// Update the state logic
void Stall::handle() {
  // Keep a small awake window after wake-up so other tasks/ISRs can run
  if (!firstSleep && (millis() - lastWakeMs) < AWAKE_WINDOW_MS) {
    return;
  }

  // Arm timer each cycle in case sleepTime is updated or a prior arm failed
  esp_err_t timerResult = esp_sleep_enable_timer_wakeup(sleepTime);
  if (timerResult != ESP_OK) {
    Serial.printf("Failed to arm timer wake-up (%d)\n", timerResult);
    return;
  }

  digitalWrite(LED_BUILTIN, LOW);
  Serial.printf("Stall State: Entering light sleep for %.1f s...\n",
                sleepTime / 1000000.0f);

  Serial.flush();
 /**esp_err_t getErr = esp_light_sleep_start();
  if (getErr != ESP_OK) {
    Serial.printf("Error entering light sleep mode: %d\n", getErr);
    return;
  }
  const esp_sleep_wakeup_cause_t cause = esp_sleep_get_wakeup_cause();
  Serial.printf("Stall wake cause: %d\n", static_cast<int>(cause));
  digitalWrite(LED_BUILTIN, HIGH);
  delay(200);  // So the other core can read sensors and then see if
               // the turbine is still on stall

  // Mark wake time to honor the awake window before re-entering sleep
  firstSleep = false;
  lastWakeMs = millis();
  */
}

// Called when exiting the state
void Stall::onExit() {
  Serial.println("Exiting Stall State...");
  // Disable wake sources configured for this state
  // esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_TIMER);
  // gpio_wakeup_disable((gpio_num_t)RIGHT_BUTTON);
  // gpio_wakeup_disable((gpio_num_t)LEFT_BUTTON);
  // esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_GPIO);
}

// Reset function (not used in this state)
void Stall::reset() {
  // No specific reset actions needed for Stall state
}
