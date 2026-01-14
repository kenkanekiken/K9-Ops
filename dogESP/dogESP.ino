#include "led_function.h"
#include "gps_module.h"
#include "dht_module.h"
#include "lora_module.h"
#include "ble_module.h"
#include "mpu.h"
#include "buzzer.h"
#include "battery.h"
#include <WiFi.h>
#include <Wire.h>

// ===== WiFi =====
// const char* ssid = "K&K";
// const char* password = "Songbird7108";
// const char* ssid = "Jun Leis S23+";
// const char* password = "lmaoooooo";
const char* ssid = "kenkanekiken";
const char* password = "12345678";

void wifiInit(void) {
  WiFi.begin(ssid, password);
  Serial.print("WiFi connecting");
  if (WiFi.status() != WL_CONNECTED) {
    delay(400);
    Serial.print("Wifi not connected");
  }
  Serial.println("\nWiFi connected");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
}

// Long-press detector
bool powerButtonLongPressed() {
  static uint32_t pressedAt = 0;
  bool pressed = (digitalRead(POWER_BTN) == LOW);

  if (pressed && pressedAt == 0) pressedAt = millis();
  if (!pressed) pressedAt = 0;

  return (pressedAt && (millis() - pressedAt >= 2000));
}

void setup() {
  Serial.begin(115200);

  batteryInit();     
  gpsInit();         
  mpuInit();        
  dhtInit();          
  loraInit();   
  // No Wifi init here   
  pinMode(POWER_BTN, INPUT_PULLUP);
  Serial.println("[DOG] Boot OK");     
}

void loop() {
  // 1️⃣ Keep modules updated (always)
  gpsUpdate();
  batteryUpdate();
  dhtUpdate();
  mpuUpdate();

  // 2️⃣ CHECK FOR SOFTWARE POWER-OFF (LONG PRESS YOU CONTROL)
  if (powerButtonLongPressed()) {
    // 🔥 Mark device as going offline
    batteryPreparePowerOff();

    // Take FINAL snapshots
    BatterySnapshot battery = batteryGetSnapshot();
    GpsSnapshot gps = gpsGetSnapshot();
    MpuSnapshot mpu = mpuGetSnapshot();

    // Send ONE LAST packet (alive = false inside battery snapshot)
    loraSendSnapshot(
      battery, gps,
      dhtGetTemperature(),
      mpu.ax, mpu.ay, mpu.az,
      (int)mpu.motion,
      mpu.state,
      mpu.steps,
      gps.speedKmh,
      gps.totalDistanceMeters
    );

    delay(150);        // let LoRa finish TX
    PMU.shutdown();    // 🔥 HARD POWER OFF (ESP32 dies here)

    while (1) {}       // safety: should never reach here
  }

  // 3️⃣ NORMAL PERIODIC SEND (every 4s)
  static uint32_t lastSend = 0;
  if (millis() - lastSend >= 4000) {
    lastSend = millis();

    BatterySnapshot battery = batteryGetSnapshot();
    GpsSnapshot gps = gpsGetSnapshot();
    MpuSnapshot mpu = mpuGetSnapshot();

    loraSendSnapshot(
      battery, gps,
      dhtGetTemperature(),
      mpu.ax, mpu.ay, mpu.az,
      (int)mpu.motion,
      mpu.state,
      mpu.steps,
      gps.speedKmh,
      gps.totalDistanceMeters
    );
  }
}