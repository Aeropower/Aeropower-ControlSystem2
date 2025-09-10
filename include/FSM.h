#include "EmergencyStop.h"
#include "PitchControlState.h"
#include "Stall.h"
#include "TorqueControl.h"
#include "state.h"
#include <ESP32Servo.h>

class FSM {
 protected:
  State* currentState = nullptr;

  PitchControlState pitchControlState;

 public:
  // Constructor that receives a Servo reference
  explicit FSM(Servo& bladesServo)
      : pitchControlState(bladesServo)  // Initialize pitchControlState correctly
  {
    currentState = &pitchControlState;
    currentState->onEnter();
  }

  void handle() {
    if (currentState) {
      currentState->handle();

      if (currentState->hasFinished()) {
        currentState->onExit();
        // Add state transition logic here
      }
    }
  }

  void reset() {
    if (currentState) {
      currentState->reset();
      currentState->onEnter();
    }
  }
};
