#include <Arduino.h>
#include <math.h>
#include <Wire.h>

#include "mpu.h"

// ================= CONFIG =================
#define MPU_ADDR       0x68
#define PWR_MGMT_1     0x6B
#define ACCEL_XOUT_H   0x3B

#define STEP_THRESHOLD      0.15f
#define MIN_STEP_INTERVAL   200

static unsigned long lastStepTime = 0;
static long stepCount = 0;
static bool aboveThreshold = false;

// Recover handling
static bool mpuOk = false;
static uint32_t lastRecoverTry = 0;

// cached snapshot
static MpuSnapshot snap = {false, 0,0,0, 0, MPU_UNKNOWN, 0};

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
  if (millis() - lastRecoverTry < 2000) return false;
  lastRecoverTry = millis();

  bool ok = mpuWrite(PWR_MGMT_1, 0x00);
  if (!ok) {
    Serial.println("[MPU] recover failed (no ACK)");
    return false;
  }

  int16_t ax, ay, az;
  ok = mpuReadAccelRaw(ax, ay, az);
  if (ok) Serial.println("[MPU] recovered OK");
  return ok;
}

// ================= SETUP =================
void mpuInit(void) {
  mpuOk = mpuWrite(PWR_MGMT_1, 0x00);
  if (!mpuOk) {
    Serial.println("[MPU] init failed (no ACK). Check wiring/power/address.");
    snap.ok = false;
    snap.state = MPU_UNKNOWN;
    return;
  }

  int16_t ax, ay, az;
  mpuOk = mpuReadAccelRaw(ax, ay, az);
  Serial.println(mpuOk ? "[MPU] READY" : "[MPU] read failed on init");

  snap.ok = mpuOk;
  snap.state = mpuOk ? MPU_STATIONARY : MPU_UNKNOWN; // start as stationary if alive
}

// ================= LOOP =================
void mpuUpdate(void) {
  if (!mpuOk) {
    mpuOk = mpuRecoverIfNeeded();
    snap.ok = mpuOk;
    return;
  }

  int16_t axRaw, ayRaw, azRaw;
  if (!mpuReadAccelRaw(axRaw, ayRaw, azRaw)) {
    Serial.println("[MPU] read failed -> offline");
    mpuOk = false;
    snap.ok = false;
    return;
  }

  float ax = axRaw / 16384.0f;
  float ay = ayRaw / 16384.0f;
  float az = azRaw / 16384.0f;

  float magnitude = sqrtf(ax * ax + ay * ay + az * az);
  float motion = fabsf(magnitude - 1.0f);

  int state = MPU_UNKNOWN;
  if (motion < 0.05f) state = MPU_STATIONARY;
  else if (motion < 0.25f) state = MPU_WALKING;
  else state = MPU_RUNNING;

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

  // update snapshot
  snap.ok = true;
  snap.ax = ax;
  snap.ay = ay;
  snap.az = az;
  snap.motion = motion;
  snap.state = state;
  snap.steps = stepCount;
}

MpuSnapshot mpuGetSnapshot(void) {
  return snap;
}