#pragma once
#define ANGLE_OF_ATTACK_DEGREES 15.0f  // degrees
#define STALL_ANGLE_DEGREES 90.0f      // degrees
#define RATED_RPM_DUTY 512.0f          // Example of Duty cycle at rated RPM
#include <ESP32Servo.h>

#include "freertos/semphr.h"

void moveServoSmooth(Servo& servo, int& currentAngle, int targetAngle, int step,
                     int delayMs);

// PWM configuration
const int pwmPin = 5;  // GPIO connected to MOSFET gate
// 50 kHz @ 10 bits overflows the available timer divider on some S3 variants.
// Drop to 20 kHz so ledcSetup can succeed while keeping decent resolution.
const int pwmFreq = 20000;     // 20 kHz switching frequency
const int pwmChannel = 0;      // High-speed channel
const int pwmResolution = 10;  // 10-bit resolution (0-1023)

//Semaphores for tasks
extern SemaphoreHandle_t xLCDSemaphore;
 