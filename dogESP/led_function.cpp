#include <Adafruit_NeoPixel.h>
#include <Arduino.h>
#include "led_function.h"

#define PIN 4
#define NUM_PIXELS 60
Adafruit_NeoPixel strip(NUM_PIXELS, PIN, NEO_GRB + NEO_KHZ800);

int g_mode =0;
int g_colorIdx =0;
int g_brightness =150;
bool needsReset = false;

const int flashSpeed = 500; // Speed for Mode 2

// Color Palette (Hex)
uint32_t colors[] = {
  0x00ABFF, // 0: Blue
  0x14FF24, // 1: Green
  0xFF0000, // 2: Red
  0xFFCD00, // 3: Yellow
  0xFFFFFF  // 4: White
};

void LedInit() {
  strip.begin();
  strip.setBrightness(g_brightness);
  strip.show();
}

void setLedProperties(int mode, int color, int brightness) {
  // This is the only function needed in loop
  g_mode = mode;
  g_colorIdx = color;
  g_brightness = brightness;

  needsReset = true;
}

void runLedAnimations() {

  if (needsReset) {
    strip.clear();
    strip.show();
    
    // We don't use delay(), we just "skip" this one frame
    // to let the voltage stabilize
    needsReset = false; 
    return; // Exit the function and come back next loop
  }
  uint32_t activeColor = colors[g_colorIdx];
  strip.setBrightness(g_brightness);

  if (g_mode == 0) {
    strip.clear();
  } 
  else if (g_mode == 1) {
    for (int i = 0; i < NUM_PIXELS; i++) strip.setPixelColor(i, activeColor);
  } 
  else if (g_mode == 2) {
    bool phase = (millis() / flashSpeed) % 2; 
    for (int i = 0; i < NUM_PIXELS; i++) {
      strip.setPixelColor(i, ((i / 4) % 2 == phase) ? activeColor : 0);
    }
  } 
  else if (g_mode == 3) {
    for (int i = 0; i < NUM_PIXELS; i++) {
      float wave = (sin((millis() / 200.0) + (i * 0.5)) + 1) / 2;
      byte r = (activeColor >> 16) & 0xFF;
      byte g = (activeColor >> 8) & 0xFF;
      byte b = activeColor & 0xFF;
      strip.setPixelColor(i, strip.Color(r * wave, g * wave, b * wave));
    }
  }
  strip.show();
}