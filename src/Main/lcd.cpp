#include "lcd.h"

#include <Arduino.h>

#include "common.h"

void IRAM_ATTR buttonRight() {
  if (taken) {
    return;
  } else {
    taken = true;
  }
  state = next(state);
  taken = false;

  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  xSemaphoreGiveFromISR(xLCDSemaphore, &xHigherPriorityTaskWoken);
  if (xHigherPriorityTaskWoken) {
    portYIELD_FROM_ISR();
  }

  return;
}

void IRAM_ATTR buttonLeft() {
  if (taken) {
    return;
  } else {
    taken = true;
  }
  state = previous(state);
  taken = false;

  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  xSemaphoreGiveFromISR(xLCDSemaphore, &xHigherPriorityTaskWoken);
  if (xHigherPriorityTaskWoken) {
    portYIELD_FROM_ISR();
  }
  return;
}

void lcd_init() {
  lcd.init();
  lcd.backlight();
  lcd.print("Initializing...");

  pinMode(RIGHT_BUTTON, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(RIGHT_BUTTON), buttonRight, FALLING);
  pinMode(LEFT_BUTTON, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(LEFT_BUTTON), buttonLeft, FALLING);
  delay(500);
}

void update_lcd() {
  Serial.println("Updating LCD...");
  Telemetry t{};
  telemetry_get_snapshot(t);
  switch (state) {
    case WIND_SPEED:
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("WIND SPEED: ");
      lcd.print(t.wind_mps);
      break;
    case RPM:
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("RPM: ");
      lcd.print(t.rpm);
      break;
    case FSM_STATE:
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("STATE: ");
      lcd.print(t.state.c_str());
      break;
    case VOLTAGE:
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("VOLTAGE: ");
      lcd.print(t.voltage);
      break;
    case ANGLE:
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("ANGLE: ");
      lcd.print(t.blade_angle);
      break;
    case CURRENT:
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("CURRENT: ");
      lcd.print(t.current);
      break;
    case POWER:
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("POWER: ");
      lcd.print(t.power);
      break;
  }
  Serial.println("LCD Updated.");
}

LCD_STATE next(LCD_STATE present) {
  int end_value = static_cast<int>(LCD_STATE::END);
  int next_value = (static_cast<int>(present) + 1) % end_value;
  return static_cast<LCD_STATE>(next_value);
}
LCD_STATE previous(LCD_STATE present) {
  int end_value = static_cast<int>(LCD_STATE::END);
  int next_value = (static_cast<int>(present) - 1 + end_value) % end_value;
  return static_cast<LCD_STATE>(next_value);
}
