#pragma once
#include <Arduino.h>
#include <stdint.h>

void dhtInit(void);
void dhtUpdate(void);        // reads sensor internally
float dhtGetTemperature(void);