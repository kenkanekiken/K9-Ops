#include <Arduino.h>
#include <math.h>
#include <Wire.h>

#include "lora_module.h"
#include "mpu.h"

// ================= CONFIG =================
#define MPU_ADDR       0x68
#define PWR_MGMT_1     0x6B
#define ACCEL_XOUT_H   0x3B

#define STEP_THRESHOLD      0.15f   // g
#define MIN_STEP_INTERVAL   200     // ms
#define MPU_UPLOAD_MS       1000    // send once per 1s

static String state = "unknown";
static unsigned long lastStepTime = 0;
static int stepCount = 0;
static bool aboveThreshold = false;
static uint32_t lastUploadTime = 0;

// Optional: if MPU disappears, don’t spam I2C forever
static bool mpuOk = false;
static uint32_t lastRecoverTry = 0;

// ================= LOW LEVEL I2C =================
static bool mpuWrite(uint8_t reg, uint8_t data) {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(reg);
  Wire.write(data);
  return (Wire.endTransmission(true) == 0);
}

static bool mpuReadAccelRaw(int16_t &ax, int16_t &ay, int16_t &az) {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(ACCEL_XOUT_H);
  if (Wire.endTransmission(false) != 0) return false;

  const int need = 6;
  int got = Wire.requestFrom(MPU_ADDR, need, true);
  if (got != need) return false;

  ax = (Wire.read() << 8) | Wire.read();
  ay = (Wire.read() << 8) | Wire.read();
  az = (Wire.read() << 8) | Wire.read();
  return true;
}

static bool mpuRecoverIfNeeded() {
  // try recover at most once per 2s
  if (millis() - lastRecoverTry < 2000) return false;
  lastRecoverTry = millis();

  // Wake MPU
  bool ok = mpuWrite(PWR_MGMT_1, 0x00);
  if (!ok) {
    Serial.println("MPU recover failed (no ACK)");
    return false;
  }

  // quick sanity read
  int16_t ax, ay, az;
  ok = mpuReadAccelRaw(ax, ay, az);
  if (ok) Serial.println("MPU recovered OK");
  return ok;
}

// ================= SETUP =================
void mpuInit(void) {
  // IMPORTANT: Wire.begin() must already be called in setup() ONCE.
  // Example in main:
  //   Wire.begin(21,22); Wire.setClock(400000); Wire.setTimeOut(50);

  mpuOk = mpuWrite(PWR_MGMT_1, 0x00);
  if (!mpuOk) {
    Serial.println("MPU6050 init failed (no ACK). Check wiring/power/address.");
    return;
  }

  // do one read to confirm
  int16_t ax, ay, az;
  mpuOk = mpuReadAccelRaw(ax, ay, az);
  Serial.println(mpuOk ? "MPU6050 ACCEL READY" : "MPU6050 read failed on init");
}

// ================= LOOP =================
void mpuRead(void) {
  if (!mpuOk) {
    // try recover sometimes
    mpuOk = mpuRecoverIfNeeded();
    return;
  }

  int16_t axRaw, ayRaw, azRaw;
  if (!mpuReadAccelRaw(axRaw, ayRaw, azRaw)) {
    Serial.println("MPU read failed -> mark offline");
    mpuOk = false;
    return;
  }

  // ±2g scale: 16384 LSB per g
  float ax = axRaw / 16384.0f;
  float ay = ayRaw / 16384.0f;
  float az = azRaw / 16384.0f;

  float magnitude = sqrtf(ax * ax + ay * ay + az * az);
  float motion = fabsf(magnitude - 1.0f);

  if (motion < 0.05f) state = "stationary";
  else if (motion < 0.25f) state = "walking";
  else state = "running";

  unsigned long now = millis();

  // Step count
  if (motion > STEP_THRESHOLD && !aboveThreshold) {
    if (now - lastStepTime > MIN_STEP_INTERVAL) {
      stepCount++;
      lastStepTime = now;
    }
    aboveThreshold = true;
  }
  if (motion < STEP_THRESHOLD * 0.5f) {
    aboveThreshold = false;
  }

  // Rate-limit LoRa TX
  if (now - lastUploadTime >= 1000) {
    lastUploadTime = now;

    // ✅ FIXED printf: format matches arguments
    Serial.printf(
      "MPU ax=%.3f ay=%.3f az=%.3f motion=%.3f state=%s steps=%d\n",
      ax, ay, az, motion, state.c_str(), stepCount
    );

    loraSendMovement(ax, ay, az, motion, state, stepCount);
  }
}