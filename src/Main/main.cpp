#include <Arduino.h>

#include "FSM.h"
#include "PitchControlState.h"

Servo myServo;
FSM fsm(myServo);  // Pass the Servo reference to FSM

void setup() {
  Serial.begin(115200);
  myServo.attach(18);  // Attach to a valid PWM pin
}

void loop() {
  fsm.handle();
  delay(100);
}
