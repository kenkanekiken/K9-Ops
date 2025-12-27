#include "led_function.h"
#include "gps_module.h"
#include "dht_module.h"
#include "lora_module.h"
#include "ble_module.h"
#include "mpu.h"
#include "buzzer.h"

void setup() {
  Serial.begin(115200);
  pinMode(2, OUTPUT);
  pmicInit();
  gpsInit();
  dhtInit();
  loraInit();
  bleInit();
  mpuInit();
  buzzerInit();
}

void loop() {
  blink_led(2);
  gpsRead();
  dhtRead();
  loraRead();
  mpuRead();
}
