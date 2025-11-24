#include <Arduino.h>
#include <ESP32Servo.h>
#include <WiFi.h>

#include "esp_bt.h"
#include "esp_wifi.h"
#include "fsm.h"
#include "gpio.h"
#include "lcd.h"
#include "sensors.h"
#include "telemetry.h"

// Authors:
// Hiram Raul Rodriguez Hernandez
// Ulises Medina Urtado
// Sergio Da Silva

#ifndef USE_PINNED_TASKS
#define USE_PINNED_TASKS 0  // set to 0 to let the scheduler pick cores
#endif

// Global objects
Servo myServo;
FSM* g_fsm = nullptr;

TaskHandle_t hSensor = nullptr;
TaskHandle_t hLCD = nullptr;
TaskHandle_t hFSM = nullptr;

// -------- Tasks --------
void SensorTask(void*) {  // Reads sensors and updates telemetry

  initTelemetry();
  analogReadResolution(12);
  sensors_init();

  for (;;) {
    sensors_poll();
    vTaskDelay(pdMS_TO_TICKS(500));  // run every 500 ms 
  }
}

void FSMTask(void*) {  // Manages the state machine
  if (g_fsm) g_fsm->initFSM();
  for (;;) {
    if (g_fsm) g_fsm->handle();
    vTaskDelay(pdMS_TO_TICKS(100));  // 100 Hz FSM
  }
}

void LCDTask(void*) {  // Updates the LCD display
  lcd_init();
  for (;;) {
    update_lcd();
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

// -------- Setup & Loop --------
void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("ESP32-S3 Turbine Control System Starting...");

  myServo.attach(SERVO_PIN);

  // Shut down WiFi and Bluetooth to reduce power consumption

  // Shut down WiFi
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  esp_wifi_stop();
  esp_wifi_deinit();

  // Shut down Bluetooth
  esp_bt_controller_mem_release(ESP_BT_MODE_BLE);
  esp_bt_controller_disable();
  esp_bt_controller_deinit();

  static FSM fsm(myServo);
  g_fsm = &fsm;

  // Tasks creation
#if USE_PINNED_TASKS
  xTaskCreatePinnedToCore(SensorTask, "SensorTask", 4096, nullptr, 2, &hSensor,
                          0);  // Core 0
  xTaskCreatePinnedToCore(LCDTask, "LCDTask", 4096, nullptr, 1, &hLCD,
                          0);  // Core 0
  xTaskCreatePinnedToCore(FSMTask, "FSMTask", 6144, nullptr, 3, &hFSM,
                          1);  // Core 1
#else
  xTaskCreate(SensorTask, "SensorTask", 4096, nullptr, 2, &hSensor);
  xTaskCreate(LCDTask, "LCDTask", 4096, nullptr, 1, &hLCD);
  xTaskCreate(FSMTask, "FSMTask", 6144, nullptr, 3, &hFSM);
#endif

  Serial.println("Tasks created");
}

void loop() {
  // Empty loop as all functionality is handled in tasks
}
