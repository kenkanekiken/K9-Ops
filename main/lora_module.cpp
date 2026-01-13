#include <Arduino.h>
#include "lora_module.h"

#include <SPI.h>
#include <LoRa.h>

#include "firebase.h"
#include <Firebase_ESP_Client.h>
extern FirebaseData fbdo;

// ---------- LoRa pins ----------
#define LORA_SCK   5
#define LORA_MISO  19
#define LORA_MOSI  27
#define LORA_CS    18
#define LORA_RST   23
#define LORA_DIO0  26   // if no packets later, try 33

// ---------- Latest received values ----------
float rxLat = 0.0f;
float rxLng = 0.0f;
bool  rxGpsOnline = false;

float rxTemp = 0.0f;
int   rxPct  = 0;
bool  rxPower = true;

float rxAx = 0.0f, rxAy = 0.0f, rxAz = 0.0f;
float rxMotion = 0.0f;
String rxState = "Unknown";
int   rxSteps = 0;

uint32_t lastPacketMs = 0;

// Upload rate-limit
static uint32_t lastFirebaseUpload = 0;
static const uint32_t FIREBASE_UPLOAD_MS = 300; // don’t spam

// CSV token helper
static String tokenAt(const String& s, int index) {
  int found = 0;
  int start = 0;
  for (int i = 0; i <= (int)s.length(); i++) {
    if (i == (int)s.length() || s[i] == ',') {
      if (found == index) return s.substring(start, i);
      found++;
      start = i + 1;
    }
  }
  return "";
}

// Upload current "latest" state to Firebase as one atomic update
static void uploadLatestToFirebase() {
  if (!Firebase.ready()) return;
  if (millis() - lastFirebaseUpload < FIREBASE_UPLOAD_MS) return;
  lastFirebaseUpload = millis();

  FirebaseJson json;

  // Always include what we have; adjust if you want “only upload when updated”
  json.set("temperature", rxTemp);
  json.set("battery", rxPct);
  json.set("power", rxPower);
  json.set("gpsStatus", rxGpsOnline);
  json.set("ax", rxAx);
  json.set("ay", rxAy);
  json.set("az", rxAz);
  json.set("motion", rxMotion);
  json.set("state", rxState);
  json.set("steps", rxSteps);

  // Only upload lat/lng if GPS is online (prevents pushing 0,0)
  if (rxGpsOnline) {
    json.set("latitude", rxLat);
    json.set("longitude", rxLng);
  }

  if (Firebase.RTDB.updateNode(&fbdo, "/devices/latest", &json)) {
    // Serial.println("[FB] latest updated");
  } else {
    Serial.print("[FB] update failed: ");
    Serial.println(fbdo.errorReason());
  }
}

static void handleMessage(const String& msg) {
  lastPacketMs = millis();

  const String type = tokenAt(msg, 0);

  if (type == "Temperature") {
    rxTemp = tokenAt(msg, 1).toFloat();
    Serial.printf("[RX] Temp = %.1f C\n", rxTemp);

    uploadLatestToFirebase();
    return;
  }

  if (type == "Power") {
    rxPower = tokenAt(msg, 1).toInt() == 1;
    Serial.printf("[RX] Power = %d\n", rxPower ? 1 : 0);

    uploadLatestToFirebase();
    return;
  }

  if (type == "Battery") {
    rxPct = tokenAt(msg, 1).toInt();
    Serial.printf("[RX] Battery = %d%%\n", rxPct);

    uploadLatestToFirebase();
    return;
  }

  if (type == "GPS") {
    rxLat = tokenAt(msg, 1).toFloat();
    rxLng = tokenAt(msg, 2).toFloat();

    Serial.printf("[RX] GPS lat=%.6f lng=%.6f\n",
                  rxLat, rxLng);

    uploadLatestToFirebase();
    return;
  }

  if (type == "GPSStatus") {
    rxGpsOnline = tokenAt(msg, 1).toInt() == 1;
    Serial.printf("[RX] GPS online = %d\n", rxGpsOnline ? 1 : 0);

    uploadLatestToFirebase();
    return;
  }
  
  // Movement,<ax>,<ay>,<az>,<motion>,<state>,<steps>
  if (type == "Movement") {
    rxAx = tokenAt(msg, 1).toFloat();
    rxAy = tokenAt(msg, 2).toFloat();
    rxAz = tokenAt(msg, 3).toFloat();
    rxMotion = tokenAt(msg, 4).toFloat();
    rxState = tokenAt(msg, 5);
    rxSteps = tokenAt(msg, 6).toInt();

    Serial.printf("[RX] Move ax=%.3f ay=%.3f az=%.3f motion=%.3f state=%s steps=%d\n", rxAx, rxAy, rxAz, rxMotion, rxState.c_str(), rxSteps);

    uploadLatestToFirebase();
    return;
  }

  Serial.print("[RX] Unknown msg: ");
  Serial.println(msg);
}

void loraInit(void) {
  SPI.begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_CS);
  LoRa.setPins(LORA_CS, LORA_RST, LORA_DIO0);

  if (!LoRa.begin(923E6)) {
    Serial.println("LoRa.begin FAILED");
    while (1) delay(1000);
  }

  Serial.println("LoRa RX init OK");
}

void loraRead(void) {
  int packetSize = LoRa.parsePacket();
  if (!packetSize) return;

  String msg;
  while (LoRa.available()) {
    msg += (char)LoRa.read();
  }

  Serial.print("LoRa RAW: ");
  Serial.println(msg);

  handleMessage(msg);

  // Optional signal quality
  Serial.printf("RSSI=%d, SNR=%.1f\n", LoRa.packetRssi(), LoRa.packetSnr());
}