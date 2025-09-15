#include <Arduino.h>

#include "FSM.h"
#include "PitchControlState.h"

Servo myServo;
FSM fsm(myServo);  // Pass the Servo reference to FSM

void setup() {
  Serial.begin(115200);
  delay(10000);
  Serial.println("ESP32-S3 Serial working setup");
  myServo.attach(18);  // Attach to a valid PWM pin
}

void loop() {
  fsm.handle();
  Serial.println("ESP32-S3 Serial working");
  delay(10000);
}
