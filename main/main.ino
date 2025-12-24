#include "led_function.h"
#include "gps_module.h"
#include "dht_module.h"

void setup() {
  Serial.begin(115200);
  pinMode(2, OUTPUT);
  pmicInit();
  gpsInit();
  dhtInit();
}

void loop() {
  blink_led(2);
  gpsRead();
  dhtRead();
}
