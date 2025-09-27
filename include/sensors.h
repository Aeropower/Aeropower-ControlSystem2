#pragma once
#include <Arduino.h>
#include "driver/pcnt.h"
#define PCNT_UNIT_USED     PCNT_UNIT_0
#define PCNT_CHANNEL_USED  PCNT_CHANNEL_0
#define PCNT_HIGH_LIMIT    65000       
#define PCNT_LOW_LIMIT     0

//API for sensor task
void sensors_poll();
void sensors_init();

