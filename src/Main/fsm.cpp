#include "./../../include/fsm.h"
/*This FSM machine changes the turbine state based on the wind speed read from
  the Modern Devices Rev. C anemometer*/
#include <Arduino.h>

#include "telemetry.h"
// Example wind speeds, the real ones must be calculated with mechanical
// division
#define CUT_IN_SPEED 1
#define RATED_SPEED 5
#define CUT_OUT_SPEED 10

FSM::FSM(Servo& bladesServo)
    : pitchControlState(bladesServo),  // Initialize pitchControlState correctly
      stallState(bladesServo),
      torqueControlState(bladesServo),
      emergencyStopState(bladesServo) {
  currentState = &stallState;
  currentState->onEnter();
}

/* Hay que verificar esto, no estoy seguro de tenerlo bien
   like, no se si siempre sera asi o la logica cambia dependiendo del estado en
   el que se encuentre*/
void FSM::handle() {
  if (!currentState) return;

  currentState->handle();
  if (!currentState->hasFinished()) return;

  // Read wind speed
  Telemetry t{};
  telemetry_get_snapshot(t);
  const float windSpeed = t.wind_mps;

  // Decide next state
  State* next = nullptr;
  if (windSpeed < CUT_IN_SPEED) {
    next = &stallState;
  } else if (windSpeed < RATED_SPEED) {
    next = &torqueControlState;
  } else if (windSpeed < CUT_OUT_SPEED) {
    next = &pitchControlState;
  } else {
    next = &emergencyStopState;
  }

  // Transition if state changed
  if (next != currentState) {
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
