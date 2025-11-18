#pragma once

class Servo {
 public:
  Servo() = default;

  int read() const { return currentAngle; }
  void write(int angle) { currentAngle = angle; }

 private:
  int currentAngle = 0;
};
