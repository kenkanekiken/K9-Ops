#include "led_function.h"
#include <Arduino.h>
#include <Adafruit_NeoPixel.h> // Ensure you have this library installed!

// ==========================
// CONFIGURATION
// ==========================
#define LED_PIN      4   // Change if your LED is on a different pin
#define NUM_LEDS     1   // Number of LEDs on your board

Adafruit_NeoPixel strip(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

// Global Variables
int currentMode = 0;       // 0=Off, 1=Solid, 2=Blink
int currentColor = 0;      // 0=Red, 1=Green, 2=Blue, 3=White
int currentBrightness = 255; 
unsigned long lastUpdate = 0;
bool blinkState = false;

void LedInit() {
    strip.begin();
    strip.show(); // Initialize all pixels to 'off'
    strip.setBrightness(255);
}

void setLedProperties(int mode, int color, int brightness) {
    currentMode = mode;
    currentColor = color;
    currentBrightness = brightness;
    strip.setBrightness(brightness);
    
    // Immediate feedback
    if (mode == 0) {
        strip.clear();
        strip.show();
    } else if (mode == 1) {
        // Solid Color Update Immediately
        runLedAnimations(); 
    }
}

uint32_t getColorHelper(int c) {
    switch(c) {
        case 0: return strip.Color(255, 0, 0);   // Red
        case 1: return strip.Color(0, 255, 0);   // Green
        case 2: return strip.Color(0, 0, 255);   // Blue
        case 3: return strip.Color(255, 255, 255); // White
        default: return strip.Color(0, 0, 0);
    }
}

void runLedAnimations() {
    if (currentMode == 0) {
        strip.clear();
        strip.show();
        return;
    }

    uint32_t color = getColorHelper(currentColor);

    // MODE 1: SOLID
    if (currentMode == 1) {
        strip.fill(color);
        strip.show();
    }
    // MODE 2: BLINK (1 second interval)
    else if (currentMode == 2) {
        if (millis() - lastUpdate > 1000) {
            lastUpdate = millis();
            blinkState = !blinkState;
            if (blinkState) strip.fill(color);
            else strip.clear();
            strip.show();
        }
    }
} 