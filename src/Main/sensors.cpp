#include "sensors.h"

#include <Arduino.h>

#include "gpio.h"
#include "telemetry.h"
#define PULSES_PER_REV 1
#define POLL_WINDOW_S 0.5f  // 500 ms

// -------- Wind logic --------
static inline float adc_to_wind(int raw) {  // Raw es el valor leido del ADC
  const float VREF = 3.3f;
  float v = (raw * VREF) / 4095.0f;  // ADC → volts
  float x = (v - 0.2f) / 0.23f;  // Formula de Modern Devices Rev. C anemometer
                                 // hay que calibrar con el datasheet
  if (x < 0) x = 0;
  return x * x * x;  // wind speed tiene un arelacion cubica
}

void sensors_init() {
  // ESP32-S3 Pulse Counter config for more info:
  // https://www.espressif.com/sites/default/files/documentation/esp32-s3_technical_reference_manual_en.pdf#pcnt
  // But is basically a digital counter embedded on the ESP32 that can be set to
  // increment or decrement on the falling or rising edges.

  pcnt_config_t cfg = {};
  cfg.pulse_gpio_num = HALL_SENSOR;
  cfg.ctrl_gpio_num = PCNT_PIN_NOT_USED;  // sin pin de control
  cfg.lctrl_mode = PCNT_MODE_KEEP;        // siguen contando normal en low
  cfg.hctrl_mode = PCNT_MODE_KEEP;        // sigue contando normal en high
  cfg.pos_mode = PCNT_COUNT_INC;          // contar en rising edge
  cfg.neg_mode = PCNT_COUNT_DIS;          // ignorar bajada
  cfg.counter_h_lim = PCNT_HIGH_LIMIT;    // <— high_limit
  cfg.counter_l_lim = PCNT_LOW_LIMIT;     // <— low_limit
  cfg.unit = PCNT_UNIT_USED;
  cfg.channel = PCNT_CHANNEL_USED;

  pinMode(HALL_SENSOR, INPUT);
  ESP_ERROR_CHECK(pcnt_unit_config(&cfg));  // <— legacy

  ESP_ERROR_CHECK(pcnt_set_filter_value(
      PCNT_UNIT_USED, 1000));  // ~12.5us hay que testear con esto
  ESP_ERROR_CHECK(pcnt_filter_enable(PCNT_UNIT_USED));
  ESP_ERROR_CHECK(pcnt_counter_pause(PCNT_UNIT_USED));
  ESP_ERROR_CHECK(pcnt_counter_clear(PCNT_UNIT_USED));
  ESP_ERROR_CHECK(
      pcnt_counter_resume(PCNT_UNIT_USED));  // Ya empieza a funcionar
}

void sensors_poll() {
  // Wind sensor
  int raw = analogRead(ANEMOMETER_PIN);
  telemetry_set_wind(adc_to_wind(raw));


  // Get hall effect sensor pulses count
  int16_t counter = 0;
  pcnt_get_counter_value(
      PCNT_UNIT_USED,
      &counter);  // guarda el valor del counter en la variable counter

  pcnt_counter_clear(PCNT_UNIT_USED);
  const float rps = ((float)counter / PULSES_PER_REV) /
                    POLL_WINDOW_S;  // Sacar las revoluciones por segundos
  const float rpm = rps * 60.0;     // Convertirlo a minutos

  telemetry_set_rpm(rpm);
}
