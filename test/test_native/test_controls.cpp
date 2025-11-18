#include "./test_mocks/Arduino.h"
#include "./test_mocks/ESP32Servo.h"
#include <unity.h>

#include "common.h"
#include "pitch_control.h"
#include "telemetry.h"
#include "torque_control.h"

// Stub Serial implementation from mocks/Arduino.h
HardwareSerial Serial;

// Minimal telemetry backing store for the included source files
Telemetry telemetry{};

void telemetry_set_wind(float wind_mps) { telemetry.wind_mps = wind_mps; }
void telemetry_set_rpm(float rpm) { telemetry.rpm = rpm; }
void telemetry_set_voltage(float voltage) { telemetry.voltage = voltage; }
void telemetry_set_current(float current) { telemetry.current = current; }
void telemetry_set_power(float power) { telemetry.power = power; }
void telemetry_set_blade_angle(int angle) { telemetry.blade_angle = angle; }
void telemetry_set_state(const std::string& state) { telemetry.state = state; }
void telemetry_get_snapshot(Telemetry& snap) { snap = telemetry; }

// Stubbed movement helper
void moveServoSmooth(Servo& servo, int& currentAngle, int targetAngle, int step,
                     int delayMs) {
  (void)step;
  (void)delayMs;
  currentAngle = targetAngle;
  servo.write(targetAngle);
}

// Local copies of the implementation so we avoid linking the MCU sources
void getPIDGains(float wind, const PID_Gains_t* lut, int size, float* Kp_out,
                 float* Ki_out) {
  if (wind <= lut[0].wind_speed_mps) {
    *Kp_out = lut[0].Kp;
    *Ki_out = lut[0].Ki;
    return;
  }
  if (wind >= lut[size - 1].wind_speed_mps) {
    *Kp_out = lut[size - 1].Kp;
    *Ki_out = lut[size - 1].Ki;
    return;
  }

  for (int i = 0; i < size - 1; i++) {
    float x0 = lut[i].wind_speed_mps;
    float x1 = lut[i + 1].wind_speed_mps;

    if (wind == x0) {
      *Kp_out = lut[i].Kp;
      *Ki_out = lut[i].Ki;
      return;
    }

    if (wind >= x0 && wind <= x1) {
      float y0_kp = lut[i].Kp;
      float y1_kp = lut[i + 1].Kp;
      float y0_ki = lut[i].Ki;
      float y1_ki = lut[i + 1].Ki;

      *Kp_out = interpolatePID(x0, x1, y0_kp, y1_kp, wind);
      *Ki_out = interpolatePID(x0, x1, y0_ki, y1_ki, wind);
      return;
    }
  }
  *Kp_out = lut[0].Kp;
  *Ki_out = lut[0].Ki;
}

void getDutyCycle(float rpm, const DutyCycle_t* lut, int size,
                  float* dutyCycle_out) {
  if (rpm <= lut[0].rpm) {
    *dutyCycle_out = lut[0].dutyCycle;
    return;
  }
  if (rpm >= lut[size - 1].rpm) {
    *dutyCycle_out = lut[size - 1].dutyCycle;
    return;
  }

  for (int i = 0; i < size - 1; i++) {
    float x0 = lut[i].rpm;
    float x1 = lut[i + 1].rpm;
    if (rpm == x0) {
      *dutyCycle_out = lut[i].dutyCycle;
      return;
    }
    if (rpm >= x0 && rpm <= x1) {
      float y0 = lut[i].dutyCycle;
      float y1 = lut[i + 1].dutyCycle;
      *dutyCycle_out = interpolateDutyCycle(x0, x1, y0, y1, rpm);
      return;
    }
  }

  *dutyCycle_out = lut[0].dutyCycle;
}

void test_interpolate_pid_linear() {
  float result = interpolatePID(5.0f, 10.0f, 1.20f, 1.60f, 7.5f);
  TEST_ASSERT_FLOAT_WITHIN(1e-5f, 1.40f, result);
}

void test_get_pid_gains_clamp_low() {
  float kp = 0, ki = 0;
  getPIDGains(-1.0f, PID_GAIN_LUT, PID_GAIN_LUT_LEN, &kp, &ki);
  TEST_ASSERT_FLOAT_WITHIN(1e-5f, 0.80f, kp);
  TEST_ASSERT_FLOAT_WITHIN(1e-5f, 0.15f, ki);
}

void test_get_pid_gains_interpolate_mid() {
  float kp = 0, ki = 0;
  getPIDGains(7.5f, PID_GAIN_LUT, PID_GAIN_LUT_LEN, &kp, &ki);
  TEST_ASSERT_FLOAT_WITHIN(1e-5f, 1.40f, kp);
  TEST_ASSERT_FLOAT_WITHIN(1e-5f, 0.275f, ki);
}

void test_interpolate_duty_cycle_linear() {
  float result = interpolateDutyCycle(0.0f, 5.0f, 0.80f, 1.60f, 2.5f);
  TEST_ASSERT_FLOAT_WITHIN(1e-5f, 1.20f, result);
}

void test_get_duty_cycle_clamp_low() {
  float duty = 0;
  getDutyCycle(-2.0f, DUTY_LUT, DUTY_CYCLE_LUT_LEN, &duty);
  TEST_ASSERT_FLOAT_WITHIN(1e-5f, 0.80f, duty);
}

void test_get_duty_cycle_interpolate_mid() {
  float duty = 0;
  getDutyCycle(2.5f, DUTY_LUT, DUTY_CYCLE_LUT_LEN, &duty);
  TEST_ASSERT_FLOAT_WITHIN(1e-5f, 1.20f, duty);
}

void test_get_duty_cycle_clamp_high() {
  float duty = 0;
  getDutyCycle(25.0f, DUTY_LUT, DUTY_CYCLE_LUT_LEN, &duty);
  TEST_ASSERT_FLOAT_WITHIN(1e-5f, 1023.0f, duty);
}

void setUp() {}
void tearDown() {}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_interpolate_pid_linear);
  RUN_TEST(test_get_pid_gains_clamp_low);
  RUN_TEST(test_get_pid_gains_interpolate_mid);
  RUN_TEST(test_interpolate_duty_cycle_linear);
  RUN_TEST(test_get_duty_cycle_clamp_low);
  RUN_TEST(test_get_duty_cycle_interpolate_mid);
  RUN_TEST(test_get_duty_cycle_clamp_high);
  return UNITY_END();
}
