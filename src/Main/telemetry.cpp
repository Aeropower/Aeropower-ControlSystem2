#include "telemetry.h"

#include <Arduino.h>  // For portENTER_CRITICAL, portEXIT_CRITICAL
Telemetry
    telemetry;  // MEMORIA COMPARTIDA ENTRE LOS DOS CORES HAY QUE TRATARLA BIEN

static portMUX_TYPE telemetryMux =
    portMUX_INITIALIZER_UNLOCKED;  // Mutex for thread safety so we don't get
                                   // corrupted data when two cores access the
                                   // struct; a mutex is a operating system
                                   // concept for more info look it up

void initTelemetry() {
  telemetry.wind_mps = 0.0;
  telemetry.rpm = 0.0;
  telemetry.voltage = 0.0;
  telemetry.current = 0.0;
  telemetry.power = 0.0;
  telemetry.blade_angle = 0;
}

void telemetry_set_wind(float wind_mps) {
  portENTER_CRITICAL(&telemetryMux);
  telemetry.wind_mps = wind_mps;
  portEXIT_CRITICAL(&telemetryMux);
}

void telemetry_set_rpm(float rpm) {
  portENTER_CRITICAL(&telemetryMux);
  telemetry.rpm = rpm;
  portEXIT_CRITICAL(&telemetryMux);
}

void telemetry_set_voltage(float voltage) {
  portENTER_CRITICAL(&telemetryMux);
  telemetry.voltage = voltage;
  portEXIT_CRITICAL(&telemetryMux);
}

void telemetry_set_current(float current) {
  portENTER_CRITICAL(&telemetryMux);
  telemetry.current = current;
  portEXIT_CRITICAL(&telemetryMux);
}

void telemetry_set_power(float power) {
  portENTER_CRITICAL(&telemetryMux);
  telemetry.power = power;
  portEXIT_CRITICAL(&telemetryMux);
}

void telemetry_set_blade_angle(int angle) {
  portENTER_CRITICAL(&telemetryMux);
  telemetry.blade_angle = angle;
  portEXIT_CRITICAL(&telemetryMux);
}
void telemetry_get_snapshot(Telemetry& snap) {
  portENTER_CRITICAL(&telemetryMux);
  snap = telemetry;
  portEXIT_CRITICAL(&telemetryMux);
}
