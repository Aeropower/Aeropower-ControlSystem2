#include <Arduino.h>
#include <ESP32Servo.h>

#include "fsm.h"
#include "gpio.h"
#include "telemetry.h"
// Objetos globales
Servo myServo;
FSM* g_fsm = nullptr;


TaskHandle_t hSensor = nullptr;
TaskHandle_t hLCD = nullptr;
TaskHandle_t hFSM = nullptr;

// -------- Wind logic --------
static inline float adc_to_wind(int raw) {//Raw es el valor leido del ADC
  const float VREF = 3.3f;
  float v = (raw * VREF) / 4095.0f;  // ADC → volts
  float x = (v - 0.2f) / 0.23f; //Formula de Modern Devices Rev. C anemometer hay que calibrar con el datasheet
  if (x < 0) x = 0;
  return x * x * x;   //wind speed tiene un arelacion cubica
}

// -------- Tasks --------
void SensorTask(void*) { //Este task lee todos los sensores y actualiza la struct telemetry
  initTelemetry();
  analogReadResolution(12);

  const TickType_t period = pdMS_TO_TICKS(100);  // 10 Hz
  for (;;) {
    int raw = analogRead(ANEMOMETER_PIN);
    telemetry_set_wind(adc_to_wind(raw));
    vTaskDelay(period);
  }
}

void FSMTask(void*) { //Maneja la máquina de estados
  Telemetry t{};
  for (;;) {
    telemetry_get_snapshot(t);
    if (g_fsm) {
      g_fsm->updateWindSpeed(t.wind_mps);
      g_fsm->handle();
    }
    vTaskDelay(pdMS_TO_TICKS(10));  // 100 Hz FSM
  }
}

void LCDTask(void*) { //Actualiza la pantalla LCD
  Telemetry t{};
  for (;;) {
    telemetry_get_snapshot(t);
    //LCD logic here
    vTaskDelay(pdMS_TO_TICKS(200));  
  }
}

// -------- Setup & Loop --------
void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("ESP32-S3 iniciado");

  myServo.attach(18);

  static FSM fsm(myServo);
  g_fsm = &fsm;

  xTaskCreatePinnedToCore(SensorTask, "SensorTask", 4096, nullptr, 2, &hSensor,
                          0);  // Core 0
  xTaskCreatePinnedToCore(LCDTask, "LCDTask", 4096, nullptr, 1, &hLCD,
                          0);  // Core 0
  xTaskCreatePinnedToCore(FSMTask, "FSMTask", 6144, nullptr, 3, &hFSM,
                          1);  // Core 1

  Serial.println("Tareas creadas");
}

void loop() {
  // vacío: todo corre en tasks
}
