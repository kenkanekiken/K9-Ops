#ifndef LED_FUNCTION_H
#define LED_FUNCTION_H

#include <Arduino.h>

void LedInit();
void setLedProperties(int mode, int color, int brightness);
void runLedAnimations();

#endif