#pragma once
#include <string>
#include <iostream>
struct Telemetry{
  float wind_mps;
  float rpm;
  float voltage;
  float current;
  float power;
  int blade_angle;
  std::string state;
};

//Global access
extern Telemetry telemetry;

void initTelemetry();

void telemetry_set_wind(float wind_mps);
void telemetry_set_rpm(float rpm);
void telemetry_set_voltage(float voltage);
void telemetry_set_current(float current);
void telemetry_set_power(float power);
void telemetry_set_blade_angle(int angle);
void telemetry_set_state(char state);
void telemetry_get_snapshot(Telemetry& snap);



