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
#define MARGIN 500

// The FSM constructor initializes the states objects and passes them the servo
FSM::FSM(Servo& bladesServo)
    : bladesServo(bladesServo),
      pitchControlState(bladesServo),
      stallState(bladesServo),
      torqueControlState(bladesServo),
      emergencyStopState(bladesServo) {}

void FSM::initFSM() {
  // Setup gpio's
  ledcSetup(pwmChannel, pwmFreq, pwmResolution);
  ledcAttachPin(pwmPin, pwmChannel);

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

  // Decide next state
  State* next = currentState;
  if (rpm <= CUT_IN_SPEED_RPM) {
    next = &stallState;
    Serial.println("FSM: currently on Stall");
  } else if (rpm <= RATED_SPEED_RPM - MARGIN) {
    next = &torqueControlState;
    Serial.println("FSM: currently on Torque");
  } else if (rpm <= RATED_SPEED_RPM) {
    next = &pitchControlState;
    Serial.println("FSM: currently on PitchControlState");
  } else {
    next = &emergencyStopState;
  }

  // Transition if state changed
  if (next != currentState) {
    Serial.printf("FSM: State changed to %s\n", next->getName().c_str());
    telemetry_set_state(next->getName());
    currentState->onExit();
    currentState = next;
    currentState->reset();
    currentState->onEnter();
  }

  currentState->handle();
}

void FSM::reset() {
  if (currentState) {
    currentState->reset();
    currentState->onEnter();
  }
}
