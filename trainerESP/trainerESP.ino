#include <Arduino.h>
#include "lora_module.h"
#include "types.h"
#include "firebase.h"
#include <XPowersLib.h>
#include <WiFi.h>
#include <Firebase_ESP_Client.h>

#define I2C_SDA 21
#define I2C_SCL 22
#define POWER_BTN 38

XPowersAXP2101 PMU;
extern FirebaseData fbdo;

static DogData dog;

// ---- rate limits (ms) ----
static uint32_t tBatt = 0, tTemp = 0, tGps = 0, tMove = 0, tSpeed = 0;
static const uint32_t INT_BATT  = 30000;
static const uint32_t INT_TEMP  = 10000;
static const uint32_t INT_GPS   =  4000;
static const uint32_t INT_MOVE  =  2000;
static const uint32_t INT_SPEED = 10000;

// ---- OFFLINE DETECTION ----
static bool dogOffline = true;
static uint32_t lastOfflinePush = 0;
static const uint32_t OFFLINE_TIMEOUT_MS = 15000;
static const uint32_t OFFLINE_PUSH_MIN_MS = 5000;

// ---- POWER BUTTON ----
static uint32_t pressedAt = 0;

// ============ WiFi =============
// const char* ssid = "K&K";
// const char* password = "Songbird7108";
// const char* ssid = "Jun Leis S23+";
// const char* password = "lmaoooooo";
const char* ssid = "kenkanekiken";
const char* password = "12345678";

void wifiInit(void) {
  WiFi.begin(ssid, password);
  Serial.print("WiFi connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
}

void powerInit(void) {
  pinMode(POWER_BTN, INPUT_PULLUP);
  Wire.begin(I2C_SDA, I2C_SCL);
  Wire.setClock(400000);
}

// ===== Firebase helper (local wrapper) =====
static const char* DOG_PATH = "/devices/dog";

static bool fbUpdateLatest(FirebaseJson &json) {
  bool ok = Firebase.RTDB.updateNode(&fbdo, DOG_PATH, &json);
  if (!ok) {
    Serial.print("[FB] update failed: ");
    Serial.println(fbdo.errorReason());
  }
  return ok;
}

// ---------------- CSV PARSER ----------------
static bool parseDogCsv(const String& line, DogData &out) {
  static char buf[300];
  if (line.length() >= sizeof(buf)) return false;
  strcpy(buf, line.c_str());

  float v[32];
  int n = 0;

  char *save = nullptr;
  char *tok = strtok_r(buf, ",", &save);
  while (tok && n < 32) {
    v[n++] = atof(tok);
    tok = strtok_r(nullptr, ",", &save);
  }

  if (n < 16) return false;

  int i = 0;
  out.seq = (uint32_t)v[i++];
  out.battPct = (int)v[i++];

  bool hasAlive = false;
  if (n >= 17) {
    int maybeAlive = (int)v[i];
    float maybeTemp = v[i + 1];
    if ((maybeAlive == 0 || maybeAlive == 1) && (maybeTemp > -40 && maybeTemp < 100)) {
      hasAlive = true;
    }
  }

  if (hasAlive) out.alive = ((int)v[i++] == 1);
  else out.alive = true;

  out.tempC = v[i++];

  out.gpsOnline = ((int)v[i++] == 1);
  out.sats = (int)v[i++];
  out.hdop = v[i++];

  out.lat = v[i++];
  out.lng = v[i++];

  out.ax = v[i++];
  out.ay = v[i++];
  out.az = v[i++];

  out.motion = v[i++];
  out.state  = (int)v[i++];
  out.steps  = (long)v[i++];

  out.speed = v[i++];
  out.distance = v[i++];

  return true;
}

// ---------------- FIREBASE RATE-LIMITED PUSH ----------------
static void pushToFirebaseRateLimited() {
  uint32_t now = millis();

  FirebaseJson base;
  base.set("seq", (int)dog.seq);
  base.set("rssi", dog.rssi);
  base.set("snr", dog.snr);
  base.set("lastSeenMs", (int)dog.lastRxMs);
  base.set("alive", dog.alive ? 1 : 0);
  fbUpdateLatest(base);

  if (now - tBatt >= INT_BATT) {
    tBatt = now;
    FirebaseJson j;
    j.set("battery/percent", dog.battPct);
    fbUpdateLatest(j);
  }

  if (now - tTemp >= INT_TEMP) {
    tTemp = now;
    FirebaseJson j;
    j.set("temperature/c", dog.tempC);
    fbUpdateLatest(j);
  }

  if (now - tGps >= INT_GPS) {
    tGps = now;
    FirebaseJson j;
    j.set("gps/online", dog.gpsOnline ? 1 : 0);
    j.set("gps/sats", dog.sats);
    j.set("gps/hdop", dog.hdop);
    j.set("gps/lat", dog.lat);
    j.set("gps/lng", dog.lng);
    fbUpdateLatest(j);
  }

  if (now - tMove >= INT_MOVE) {
    tMove = now;
    const char* stateText = "unknown";
    if (dog.state == 1) stateText = "stationary";
    else if (dog.state == 2) stateText = "walking";
    else if (dog.state == 3) stateText = "running";

    FirebaseJson j;
    j.set("imu/ax", dog.ax);
    j.set("imu/ay", dog.ay);
    j.set("imu/az", dog.az);
    j.set("imu/motion", dog.motion);
    j.set("imu/state", dog.state);
    j.set("imu/stateText", stateText);   // ✅ ADD THIS LINE
    j.set("imu/steps", (int)dog.steps);
    fbUpdateLatest(j);
  }

  if (now - tSpeed >= INT_SPEED) {
    tSpeed = now;
    FirebaseJson j;
    j.set("move/speed", dog.speed);
    j.set("move/distance", dog.distance);
    fbUpdateLatest(j);
  }
}

void setup() {
  Serial.begin(115200);
  wifiInit();
  firebaseInit();
  loraInit();
  powerInit();
  Serial.println("[TRAINER] Ready");
}

void loop() {
  // -------- POWER BUTTON (Trainer shutdown) --------
  bool pressed = (digitalRead(POWER_BTN) == LOW);

  if (pressed && pressedAt == 0) pressedAt = millis();
  if (!pressed) pressedAt = 0;

  if (pressedAt && (millis() - pressedAt >= 2000)) {
    Serial.println("Trainer powering off...");
    Firebase.RTDB.setBool(&fbdo, "/devices/trainer/power", false);
    delay(150);
    PMU.shutdown();
  }

  // -------- LoRa RX --------
  String line;
  int rssi = 0;
  float snr = 0;

  if (loraReceiveLine(line, rssi, snr)) {
    Serial.println("----- LoRa RX -----");
    Serial.println(line);
    Serial.printf("RSSI: %d | SNR: %.1f\n", rssi, snr);

    DogData parsed = dog;
    if (parseDogCsv(line, parsed)) {
      parsed.rssi = rssi;
      parsed.snr  = snr;
      parsed.lastRxMs = millis();
      dog = parsed;

      if (dogOffline) {
        dogOffline = false;
        FirebaseJson j;
        j.set("alive", 1);
        j.set("status", "online");
        fbUpdateLatest(j);
        Serial.println("[STATUS] Dog ONLINE");
      }

      pushToFirebaseRateLimited();
    }
  }

  // -------- OFFLINE TIMEOUT --------
  uint32_t now = millis();
  if (dog.lastRxMs > 0 && (now - dog.lastRxMs > OFFLINE_TIMEOUT_MS)) {
    if (!dogOffline && (now - lastOfflinePush > OFFLINE_PUSH_MIN_MS)) {
      dogOffline = true;
      lastOfflinePush = now;

      FirebaseJson j;
      j.set("alive", 0);
      j.set("status", "offline");
      j.set("gps/online", 0);
      fbUpdateLatest(j);

      Serial.println("[STATUS] Dog OFFLINE");
    }
  }
}