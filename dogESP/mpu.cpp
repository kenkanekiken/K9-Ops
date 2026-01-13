#include <Arduino.h>
#include <Wire.h>
#include <math.h>
// LoRa sender (we will call loraSend)
#include "lora_module.h"
// ================= CONFIG =================
#define MPU_ADDR 0x68  

// MPU6050 Registers
#define PWR_MGMT_1     0x6B
#define ACCEL_XOUT_H   0x3B

// ================= VARIABLES =================
// For dog movement State capture
#define SAMPLE_RATE 50
#define WINDOW_SIZE 50
// For dog step count capture
#define STEP_THRESHOLD 0.15   // g
#define MIN_STEP_INTERVAL 200 // ms (max ~5 steps/sec)
#define MPU_UPLOAD_INTERVAL 1000 


float motionBuffer[WINDOW_SIZE];
int bufferIndex = 0;
String state;
unsigned long lastStepTime = 0;
int stepCount = 0;
bool aboveThreshold = false;
static uint32_t lastUploadTime = 0;
int16_t axRaw, ayRaw, azRaw;
// ================= FUNCTIONS =================
void mpuWrite(uint8_t reg, uint8_t data) {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(reg);
  Wire.write(data);
  Wire.endTransmission();
}

bool mpuReadAccel(int16_t &ax, int16_t &ay, int16_t &az) {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(ACCEL_XOUT_H);
  if (Wire.endTransmission(false) != 0) return false;

  Wire.requestFrom(MPU_ADDR, 6, true);
  if (Wire.available() < 6) return false;

  ax = (Wire.read() << 8) | Wire.read();
  ay = (Wire.read() << 8) | Wire.read();
  az = (Wire.read() << 8) | Wire.read();

  return true;
}

// ================= SETUP =================
#define SDA_PIN 21
#define SCL_PIN 22
void mpuInit(void) {
  Wire.begin(SDA_PIN, SCL_PIN);
  // Wake up MPU60500
  mpuWrite(PWR_MGMT_1, 0x00);
  Serial.println("MPU6050 ACCEL ONLY READY");
}

// ================= LOOP =================
void mpuRead(void) {
  if (mpuReadAccel(axRaw, ayRaw, azRaw)) {

    // ±2g scale → 16384 LSB per g
    float ax = axRaw / 16384.0;
    float ay = ayRaw / 16384.0;
    float az = azRaw / 16384.0;

    float magnitude = sqrt(ax * ax + ay * ay + az * az);

    float motion = fabs(magnitude - 1.0); // minus one to account for gravity

    if (motion < 0.05) {
      state = "stationary";
    }
    else if(motion < 0.25) {
      state = "walking";
    }
    else {
      state = "running";
    }

    unsigned long now = millis();

    if (motion > STEP_THRESHOLD && !aboveThreshold) {

    if (now - lastStepTime > MIN_STEP_INTERVAL) {
    stepCount++;
    lastStepTime = now;
      }
    aboveThreshold = true;
    }

    if (motion < STEP_THRESHOLD * 0.5) {
      aboveThreshold = false;
    }

    if (millis() - lastUploadTime >= MPU_UPLOAD_INTERVAL) {
      lastUploadTime = millis();

      Serial.printf("MPU ax=%.3f ay=%.3f az=%.3f motion=%.3f state=%s steps=%d\n",
                  ax, ay, az, motion, state.c_str(), stepCount);

      loraSendMovement(ax, ay, az, motion, state, stepCount);
    }
  }
}