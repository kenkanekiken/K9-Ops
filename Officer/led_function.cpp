#include <Arduino.h>
#include "led_function.h"

unsigned long lastTime = 0;
const unsigned long interval = 1000;
bool flag = false;

void blink_led(int pin) {
  unsigned long now = millis();

  if (now - lastTime >= interval) {
    lastTime = now;
    digitalWrite(pin, flag);
    flag = !flag;
  }
}