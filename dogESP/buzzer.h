#ifndef BUZZER_H
#define BUZZER_H

#include <Arduino.h>

// CHANGED TO PIN 25 FOR T-BEAM
#define BUZZER_PIN 25 

void initBuzzer();
void buzzerOn();
void buzzerOff();

#endif