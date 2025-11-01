#include "fsm.h"
/*This FSM machine changes the turbine state based on the wind speed read from
  the Modern Devices Rev. C anemometer*/
#include <Arduino.h>

#include "telemetry.h"
#include "common.h"
// Example wind speeds, the real ones must be calculated with mechanical
// division
#define CUT_IN_SPEED_RPM 120  // Example: 2 m/s; below that turbine is on stall
#define RATED_SPEED_RPM 240
// Example: 4 m/s below that and greater than CUT_IN_SPEED turbine is on
// MPPT(torque)
#define CUT_OUT_SPEED_RPM 360
// Example: 6 m/s; below that turbine is on pitch control, greater than
// that turbine must enter on emergency mode

// The FSM constructor initializes the states objects and passes them the servo
FSM::FSM(Servo& bladesServo)
    : pitchControlState(bladesServo),
      stallState(bladesServo),
      torqueControlState(bladesServo),
      emergencyStopState(bladesServo) {
}

void FSM::initFSM() {
  // Setup gpio's
  ledcSetup(pwmChannel, pwmFreq, pwmResolution);
  ledcAttachPin(pwmPin, pwmChannel);

  // Start on Stall
  currentState = &stallState;
  currentState->onEnter();
}
  void FSM::handle() {
    if (!currentState) return;

    currentState->handle();
    // if (!currentState->hasFinished()) return;

    // Read rpm from telemetry
    Telemetry t{};
    telemetry_get_snapshot(t);
    const float rpm = t.rpm;

    // Decide next state
    State* next = nullptr;
    if (rpm <= CUT_IN_SPEED_RPM) {
      next = &stallState;
      Serial.println("FSM: currently on Stall");
    } else if (rpm <= RATED_SPEED_RPM) {
      next = &torqueControlState;
      Serial.println("FSM: currently on Torque");
    } else if (rpm <= CUT_OUT_SPEED_RPM) {
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
  }

  void FSM::reset() {
    if (currentState) {
      currentState->reset();
      currentState->onEnter();
    }
  }
