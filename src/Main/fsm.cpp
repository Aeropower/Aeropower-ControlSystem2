#include "fsm.h"
/*This FSM machine changes the turbine state based on the wind speed read from
  the Modern Devices Rev. C anemometer*/
#include <Arduino.h>

#include "common.h"
#include "telemetry.h"
// Example wind speeds, the real ones must be calculated with mechanical
// division
#define CUT_IN_SPEED_RPM 150  // Example: 2 m/s; below that turbine is on stall
#define RATED_SPEED_RPM 2000
// Hysteresis thresholds (Schmitt trigger style) to avoid chattering
#define HYST_STALL_EXIT_RPM 200        // Stall -> Torque (upwards)
#define HYST_STALL_ENTRY_RPM 120       // Torque -> Stall (downwards)
#define HYST_TORQUE_TO_PITCH_RPM 1900  // Torque -> Pitch (upwards)
#define HYST_PITCH_TO_TORQUE_RPM 1700  // Pitch -> Torque (downwards)
#define HYST_PITCH_TO_EMERG_RPM 2200   // Pitch -> Emergency (upwards)

FSM* FSM::instance = nullptr;

void IRAM_ATTR FSM::onTimerISR() {
  if (instance) {
    instance->transition_ready = true;
  }
}

// The FSM constructor initializes the states objects and passes them the servo
FSM::FSM(Servo& bladesServo)
    : bladesServo(bladesServo),
      pitchControlState(bladesServo),
      stallState(bladesServo),
      torqueControlState(bladesServo),
      emergencyStopState(bladesServo) {
  instance = this;
}

void FSM::initFSM() {
  // Setup gpio's
  ledcSetup(pwmChannel, pwmFreq, pwmResolution);
  ledcAttachPin(pwmPin, pwmChannel);

  // One-shot timer setup: 1 MHz base (80 MHz / 80), armed on demand
  timer = timerBegin(0, 80, true);
  if (timer) {
    timerAttachInterrupt(timer, &FSM::onTimerISR, true);
    // 3 s one-shot (autoreload = false), disarmed until needed
    timerAlarmWrite(timer, 3000000, false);
    timerAlarmDisable(timer);
    timerWrite(timer, 0);
  } else {
    Serial.println("Timer initialization failed!");
  }

  int currentAngle = bladesServo.read();
  moveServoSmooth(bladesServo, currentAngle, 0, 1,
                  10);  // Ensure servo is at 0 degrees at start
  // Start on Stall
  currentState = &stallState;
  currentState->onEnter();
}
void FSM::handle() {
  if (!currentState) return;

  // Read rpm from telemetry
  Telemetry t{};
  telemetry_get_snapshot(t);
  const float rpm = t.rpm;

  // Decide next state with hysteresis thresholds
  State* next = currentState;
  if (currentState == &stallState) {
    if (rpm >= HYST_STALL_EXIT_RPM) {
      next = &torqueControlState;
      Serial.println("FSM: leaving Stall -> Torque");
    }
  } else if (currentState == &torqueControlState) {
    if (rpm <= HYST_STALL_ENTRY_RPM) {
      next = &stallState;
      Serial.println("FSM: Torque -> Stall");
    } else if (rpm >= HYST_TORQUE_TO_PITCH_RPM) {
      next = &pitchControlState;
      Serial.println("FSM: Torque -> Pitch");
    }
  } else if (currentState == &pitchControlState) {
    if (rpm >= HYST_PITCH_TO_EMERG_RPM) {
      next = &emergencyStopState;
      Serial.println("FSM: Pitch -> Emergency");
    } else if (rpm <= HYST_PITCH_TO_TORQUE_RPM) {
      next = &torqueControlState;
      Serial.println("FSM: Pitch -> Torque");
    }
  } else if (currentState == &emergencyStopState) {
    next = &emergencyStopState;  // Stay until reset
  }

  // If no state change needed, cancel pending timer/flag and handle state
  if (next == currentState) {
    transition_ready = false;
    if (timer) {
      timerAlarmDisable(timer);
      timerWrite(timer, 0);
    }
    currentState->handle();
    return;
  }

  // First detection: arm one-shot timer
  if (!transition_ready) {
    if (timer) {
      timerWrite(timer, 0);
      timerAlarmEnable(timer);
    }
    // Defer transition until timer elapses
    currentState->handle();
    return;
  }

  // Timer elapsed and condition still holds: perform transition
  transition_ready = false;
  if (timer) {
    timerAlarmDisable(timer);
    timerWrite(timer, 0);
  }

  Serial.printf("FSM: State changed to %s\n", next->getName().c_str());
  telemetry_set_state(next->getName());
  currentState->onExit();
  currentState = next;
  currentState->reset();
  currentState->onEnter();

  currentState->handle();
}

void FSM::reset() {
  if (currentState) {
    currentState->reset();
    currentState->onEnter();
  }
}
