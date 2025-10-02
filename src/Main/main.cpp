#include <Arduino.h>
#include <ESP32Servo.h>
#include <WiFi.h>

#include "esp_bt.h"
#include "esp_wifi.h"
#include "fsm.h"
#include "gpio.h"
#include "sensors.h"
#include "telemetry.h"
#include "lcd.h"
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
  sensors_init();

  const TickType_t period = pdMS_TO_TICKS(10);  // 5 Hz
  for (;;) {
    sensors_poll();
    vTaskDelay(period);
  }
}

void FSMTask(void*) {  // Maneja la máquina de estados
  for (;;) {
    if (g_fsm) g_fsm->handle();
    vTaskDelay(pdMS_TO_TICKS(100));  // 100 Hz FSM
  }
}

void LCDTask(void*) {
  lcd_init();
  for (;;) {
    update_lcd();
    vTaskDelay(pdMS_TO_TICKS(100));
  }
}

// -------- Setup & Loop --------
void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("ESP32-S3 iniciado");

  myServo.attach(SERVO_PIN);

  // Apagar wifi y bluetooth para disminuir el consumo de potencia
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  esp_wifi_stop();
  esp_wifi_deinit();

  // Apaga y libera toda la RAM reservada para BLE
  esp_bt_controller_mem_release(ESP_BT_MODE_BLE);
  esp_bt_controller_disable();
  esp_bt_controller_deinit();

  static FSM fsm(myServo);
  g_fsm = &fsm;

  xTaskCreatePinnedToCore(SensorTask, "SensorTask", 4096, nullptr, 3, &hSensor,
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
