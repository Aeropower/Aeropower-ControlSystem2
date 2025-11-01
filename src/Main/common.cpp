#include "common.h"

void moveServoSmooth(Servo &servo, int &currentAngle, int targetAngle, int step, int delayMs){
    if(currentAngle < targetAngle){
      for(int angle = currentAngle; angle <= targetAngle; angle += step){
          servo.write(angle);
          delay(delayMs);
      }
      currentAngle = targetAngle;
    }

    else if(currentAngle > targetAngle){
      for(int angle = currentAngle; angle >= targetAngle; angle -= step){
          servo.write(angle);
          delay(delayMs);
      }
      currentAngle = targetAngle;
    }
}
