#include "sensors.h"

#include <Arduino.h>

#include "gpio.h"
#include "telemetry.h"
#define PULSES_PER_REV 1
#define POLL_WINDOW_S 0.5f  // 500 ms

// -------- Wind logic --------
static inline float get_wind_speed() {  // Raw es el valor leido del ADC
                                        // 1) Leer ADC (ESP32)
  int rawTMP = analogRead(ANEMOMETER_TMP);
  int rawRV = analogRead(ANEMOMETER_RV);

  // 2) Convertir a voltios en el pin, y luego deshacer divisor -> voltios
  // reales del sensor (a 5 V)
  float VpinTMP = adc_to_vpin(rawTMP);
  float VpinRV = adc_to_vpin(rawRV);

  float VsrcTMP = vpin_to_vsrc(VpinTMP);  // voltaje “real” que vería el sensor
                                          // si lo midieras sin divisor
  float VsrcRV = vpin_to_vsrc(VpinRV);

  // 3) Para fórmulas originales que usan CUENTAS de 10 bits / 5V, convertimos
  // Vsrc a “AD units” equivalentes
  int TMP_Therm_ADunits = (int)roundf(vsrc_to_counts10(VsrcTMP));
  int RV_Wind_ADunits =
      vsrc_to_counts10(VsrcRV);  // si lo necesitas en cuentas, aquí está

  // 4) Para fórmulas originales en VOLTIOS, usa Vsrc* (no Vpin)
  float RV_Wind_Volts = VsrcRV;

  // === MISMAS FÓRMULAS del sketch de Modern Device ===
  // TempC*100 y zeroWind usan ADunits (equivalentes a 10 bits / 5V)
  int TempCtimes100 =
      (0.005f * ((float)TMP_Therm_ADunits * (float)TMP_Therm_ADunits)) -
      (16.862f * (float)TMP_Therm_ADunits) + 9075.4f;

  int zeroWind_ADunits =
      -0.0006f * ((float)TMP_Therm_ADunits * (float)TMP_Therm_ADunits) +
      1.0727f * (float)TMP_Therm_ADunits + 47.172f;

  // zeroWind_volts también en voltios REALES del sensor (5V) → usa factor
  // 5/1023
  float zeroWind_volts =
      (zeroWind_ADunits * (5.0f / 1023.0f)) - zeroWindAdjustment;

  // Velocidad: usa Vsrc (voltaje real del sensor), no Vpin
  float WindSpeed_MPH =
      powf(((RV_Wind_Volts - zeroWind_volts) / 0.2300f), 2.7265f);

  // === Salida ===
  Serial.print("  TMP volts(pin) ");
  Serial.print(VpinTMP, 3);
  Serial.print("  TMP volts(src) ");
  Serial.print(VsrcTMP, 3);

  // Serial.print("  RV volts(pin) ");
  // Serial.print(VpinRV, 3);
  // Serial.print("  RV volts(src) ");
  // Serial.print(RV_Wind_Volts, 3);

  // Serial.print("\t  TempC*100 ");
  // Serial.print(TempCtimes100);

  Serial.print("   ZeroWind volts ");
  Serial.print(zeroWind_volts, 3);

  Serial.print("   WindSpeed MPH ");
  Serial.println(WindSpeed_MPH, 2);

  return WindSpeed_MPH;
}

// Test Wind Logic
int sineWindfunc() {
  float t = millis() / 1000.0f;

  // Sine wave value, range [-1, 1]
  float sineVal = sinf(omega * t);

  // Scale if you want e.g. [-100, 100]
  float scaled = 100.0f * sineVal;

  Serial.println(scaled);

  delay(50);  // print every 50 ms
  return scaled;
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
  pinMode(ANEMOMETER_TMP, INPUT);
  pinMode(ANEMOMETER_RV, INPUT);
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
  // Serial.println("sensor");
  // Wind sensor
  telemetry_set_wind(get_wind_speed());  // Normally we use this
  // telemetry_set_wind(sineWindfunc());

  // Get hall effect sensor pulses count
  int16_t counter = 0;
  pcnt_get_counter_value(
      PCNT_UNIT_USED,
      &counter);  // guarda el valor del counter en la variable counter

  pcnt_counter_clear(PCNT_UNIT_USED);
  const float rps = ((float)counter / PULSES_PER_REV) /
                    POLL_WINDOW_S;  // Sacar las revoluciones por segundos
  const float rpm = rps * 60.0;     // Convertirlo a minutos

  telemetry_set_rpm(rpm);  // Normally we use this one
  // telemetry_set_rpm(sineWindfunc());  // Testing purposes
}
