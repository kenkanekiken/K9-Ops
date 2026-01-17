#include <Arduino.h>
#include "led_function.h"

bool isBlinking = false;

void blink_led(int pin) {
  if (isBlinking == false) {
    digitalWrite(pin, LOW);
    return;
  }
  unsigned long now = millis();
  static unsigned long lastTime = 0;
  const unsigned long interval = 1000;
  static bool flag = false;

  if (now - lastTime >= interval) {
    lastTime = now;
    digitalWrite(pin, flag);
    flag = !flag;
  }
}