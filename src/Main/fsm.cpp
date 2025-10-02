#include "fsm.h"
/*This FSM machine changes the turbine state based on the wind speed read from
  the Modern Devices Rev. C anemometer*/
#include <Arduino.h>

#include "telemetry.h"
// Example wind speeds, the real ones must be calculated with mechanical
// division
#define CUT_IN_SPEED 2  // Example: 2 m/s; below that turbine is on stall
#define RATED_SPEED  4
   // Example: 4 m/s below that and greater than CUT_IN_SPEED turbine is on
   // MPPT(torque)
#define CUT_OUT_SPEED 6
   // Example: 6 m/s; below that turbine is on pitch control, greater than
   // that turbine must enter on emergency mode

   //The FSM constructor initializes the states objects and passes them the servo
    FSM::FSM(Servo& bladesServo)
    : pitchControlState(bladesServo),  
      stallState(bladesServo),
      torqueControlState(bladesServo),
      emergencyStopState(bladesServo) {
  currentState = &stallState;
  currentState->onEnter();
}


void FSM::handle() {
  if (!currentState) return;

  currentState->handle();
  // if (!currentState->hasFinished()) return;

  // Read wind speed
  Telemetry t{};
  telemetry_get_snapshot(t);
  const float windSpeed = t.wind_mps;

  // Decide next state
  State* next = nullptr;
  if (windSpeed < CUT_IN_SPEED) {
    next = &stallState;
    Serial.println("FSM: switching to Stall");
  } else if (windSpeed < RATED_SPEED) {
    next = &torqueControlState;
    Serial.println("FSM: switching to Torque");
  } else if (windSpeed < CUT_OUT_SPEED) {  ///// <
    next = &pitchControlState;
    Serial.println("FSM: switching to PitchControlState");
  } else {
    next = &emergencyStopState;
  }

  // Transition if state changed
  if (next != currentState) {
    Serial.println("FSM: Actual swicth");
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
