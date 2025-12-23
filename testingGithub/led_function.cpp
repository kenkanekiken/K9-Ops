#include "led_function.h"
#include <Arduino.h>

void blink_led(int pin) {
  digitalWrite(pin, HIGH);
  delay(300);
  digitalWrite(pin, LOW);
  delay(300);
}