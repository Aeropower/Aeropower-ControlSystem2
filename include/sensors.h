#pragma once
#include <Arduino.h>
#include "driver/pcnt.h"
#define PCNT_UNIT_USED     PCNT_UNIT_0
#define PCNT_CHANNEL_USED  PCNT_CHANNEL_0
#define PCNT_HIGH_LIMIT   32767
#define PCNT_LOW_LIMIT   -32768

static inline float get_wind_speed();
int sineWindfunc();
//API for sensor task
void sensors_poll();
void sensors_init();


// === Divisor: Rt=100k en serie (arriba), Rb=470k||220k (abajo) ===
static const float RT = 100000.0f;
static const float RB = 1.0f / (1.0f/470000.0f + 1.0f/220000.0f);  // ≈149860 Ω
static const float DIV_RATIO = RB / (RT + RB);     // ≈ 0.5994 (Vpin = DIV_RATIO * Vsensor)
static const float UNDIVER   = 1.0f / DIV_RATIO;   // ≈ 1.668  (Vsensor = Vpin * UNDIVER)

// === ADC del ESP32 ===
static const float VREF    = 3.3f;   // referencia típica
static const int   ADC_MAX = 4095;   // 12 bits

// === Ajuste de cero del fabricante (igual que el sketch original) ===
const float zeroWindAdjustment = 0.4;   // vuelve a calibrar con el “vaso” si hace falta
//const float zeroWindAdjustment =  -0.5; // negative numbers yield smaller wind speeds and vice versa.

extern int TMP_Therm_ADunits;  //temp termistor value from wind sensor
extern float RV_Wind_ADunits;    //RV output from wind sensor 
extern float RV_Wind_Volts;
extern unsigned long lastMillis;
extern int TempCtimes100;
extern float zeroWind_ADunits;
extern float zeroWind_volts;
extern float WindSpeed_MPH;

// Helpers
static inline float adc_to_vpin(int raw)         { return (raw * VREF) / ADC_MAX; }      // V en el pin
static inline float vpin_to_vsrc(float vpin)     { return vpin * UNDIVER; }              // V reales del sensor (5 V rail)
static inline float vsrc_to_counts10(float vsrc) { return vsrc * (1023.0f / 5.0f); }     // cuentas equivalentes 10 bits/5 V

//Test Variables
const float freq = 0.1f;     
const float omega = 2.0f * M_PI * freq;
