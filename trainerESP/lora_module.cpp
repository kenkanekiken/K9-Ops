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

// Firebase base path you want
static const char* DOG_BASE = "/devices/dog";

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

float rxMeter = 0.0f;
float rxKm = 0.0f;
float rxSpeed = 0.0f;

uint32_t lastPacketMs = 0;

// Upload rate-limit
static uint32_t lastFirebaseUpload = 0;
static const uint32_t FIREBASE_UPLOAD_MS = 250; // don’t spam too hard

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

// Upload current state to Firebase at /devices/dog/*
static void uploadDogToFirebase() {
  if (!Firebase.ready()) return;
  if (millis() - lastFirebaseUpload < FIREBASE_UPLOAD_MS) return;
  lastFirebaseUpload = millis();

  FirebaseJson update;

  // /devices/dog/temperature
  update.set("temperature", rxTemp);

  // /devices/dog/battery , /devices/dog/power
  update.set("battery", rxPct);
  update.set("power", rxPower);

  // /devices/dog/gps/*
  update.set("gps/gpsOnline", rxGpsOnline);
  if (rxGpsOnline) {
    update.set("gps/latitude", rxLat);
    update.set("gps/longitude", rxLng);
    // /devices/dog/velocity/*
    update.set("velocity/distanceMeter", rxMeter);
    update.set("velocity/distanceKm", rxKm);
    update.set("velocity/speed", rxSpeed);
  }

  // /devices/dog/movement/*
  update.set("movement/ax", rxAx);
  update.set("movement/ay", rxAy);
  update.set("movement/az", rxAz);
  update.set("movement/motion", rxMotion);
  update.set("movement/state", rxState);
  update.set("movement/steps", rxSteps);

  if (Firebase.RTDB.updateNode(&fbdo, DOG_BASE, &update)) {
    Serial.println("[Firebase] /devices/dog updated");
  } else {
    Serial.print("[Firebase] update failed: ");
    Serial.println(fbdo.errorReason());
  }
}

static void handleMessage(const String& msg) {
  lastPacketMs = millis();

  const String type = tokenAt(msg, 0);

  if (type == "Temperature") {
    rxTemp = tokenAt(msg, 1).toFloat();
    Serial.printf("[RX] Temp = %.1f C\n", rxTemp);
    uploadDogToFirebase();
    return;
  }

  if (type == "Power") {
    rxPower = tokenAt(msg, 1).toInt() == 1;
    Serial.printf("[RX] Power = %d\n", rxPower ? 1 : 0);
    uploadDogToFirebase();
    return;
  }

  if (type == "Battery") {
    rxPct = tokenAt(msg, 1).toInt();
    Serial.printf("[RX] Battery = %d%%\n", rxPct);
    uploadDogToFirebase();
    return;
  }

  // GPS,<lat>,<lng>
  if (type == "GPS") {
    rxLat = tokenAt(msg, 1).toFloat();
    rxLng = tokenAt(msg, 2).toFloat();
    Serial.printf("[RX] GPS lat=%.6f lng=%.6f\n", rxLat, rxLng);
    uploadDogToFirebase();
    return;
  }

  // GPSStatus,<0/1>
  if (type == "GPSStatus") {
    rxGpsOnline = tokenAt(msg, 1).toInt() == 1;
    Serial.printf("[RX] GPS online = %d\n", rxGpsOnline ? 1 : 0);
    uploadDogToFirebase();
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

    Serial.printf("[RX] Movement ax=%.3f ay=%.3f az=%.3f motion=%.3f state=%s steps=%d\n",
                  rxAx, rxAy, rxAz, rxMotion, rxState.c_str(), rxSteps);

    uploadDogToFirebase();
    return;
  }

  // Velocity,<Meter>,<Km>,<Speed>
  if (type == "Velocity") {
    rxMeter = tokenAt(msg, 1).toFloat();
    rxKm = tokenAt(msg, 2).toFloat();
    rxSpeed = tokenAt(msg, 3).toFloat();

    Serial.printf("[RX] Velocity Meter=%.2f Km=%.2f Speed=%.2f\n",
                  rxMeter, rxKm, rxSpeed);

    uploadDogToFirebase();
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

  Serial.printf("RSSI=%d, SNR=%.1f\n", LoRa.packetRssi(), LoRa.packetSnr());
}