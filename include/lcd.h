#pragma once
#include <Arduino.h>
#include <LiquidCrystal_I2C.h>

#include "gpio.h"
#include "telemetry.h"

// Global
/*This will be an integer variable with constraints that has to  wrap around
in case of an overflow, but basicalle every time the buttom left activates its
interrupt it will substract 1 from the variable and everytime the right button
activates its interrupt it will sum 1. The integer in the variable represent a
state defined by the number of data we want to show at once.
*/
enum LCD_STATE {

  WIND_SPEED,
  RPM,
  FSM_STATE,
  VOLTAGE,
  ANGLE,
  CURRENT,
  POWER,
  END  // This has to be always at the end it serves as a ending int to traverse
       // the states

};

LiquidCrystal_I2C lcd(I2C_ADDR, I2C_NUM_COL,
                      I2C_NUM_ROWS);  // I2C address 0x27 for the LCD
LCD_STATE state;
bool taken = false;

LCD_STATE next(LCD_STATE present);
LCD_STATE previous(LCD_STATE present);

void lcd_init();
void IRAM_ATTR buttonLeft();
void IRAM_ATTR buttonRight();
void update_lcd();
