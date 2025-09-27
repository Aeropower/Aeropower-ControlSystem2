#include <Arduino.h>
#include <ESP32Servo.h>

#include "fsm.h"
#include "gpio.h"
#include "telemetry.h"
#include "sensors.h"
// Objetos globales
Servo myServo;
FSM* g_fsm = nullptr;

TaskHandle_t hSensor = nullptr;
TaskHandle_t hLCD = nullptr;
TaskHandle_t hFSM = nullptr;


// -------- Tasks --------
void SensorTask(void*) {  // Este task lee todos los sensores y actualiza la
                          // struct telemetry
  initTelemetry();
  analogReadResolution(12);

  const TickType_t period = pdMS_TO_TICKS(500);  // 5 Hz
  for (;;) {
    sensors_poll();
    vTaskDelay(period);
  }
}

void FSMTask(void*) {  // Maneja la máquina de estados
  for (;;) {
    if (g_fsm) g_fsm->handle();
    vTaskDelay(pdMS_TO_TICKS(10));  // 100 Hz FSM
  }
}

void LCDTask(void*) {  
  Telemetry t{};
  for (;;) {
    telemetry_get_snapshot(t);
    // LCD logic here
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
