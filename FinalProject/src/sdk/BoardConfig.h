#pragma once

#include <Arduino.h>

// ==============================================================================
// HANDHELD HARDWARE & SIMULATOR CONFIGURATION
// ==============================================================================
// Select target via PlatformIO environment:
//   - pio run -e wokwi     (defines TARGET_WOKWI=1)
//   - pio run -e hardware  (defines TARGET_HARDWARE=1)

// Display dimensions (landscape)
#define SCREEN_WIDTH  320
#define SCREEN_HEIGHT 240

// SPI Display Pins (ESP32-S3)
#define TFT_CS    5
#define TFT_DC    4
#define TFT_SCK   12
#define TFT_MOSI  11
#define TFT_MISO  -1

#if defined(TARGET_WOKWI)
  #define TFT_RST       -1   // Wokwi diagram ties RST to GPIO 14 or ESP reset
  #define TFT_ROTATION  1    // Landscape orientation for Wokwi ILI9341
  #define TFT_SPI_FREQ  16000000

  // Input Button Pins (Active-LOW, matching FinalProject/diagram.json)
  #define PIN_UP      45
  #define PIN_DOWN    2
  #define PIN_LEFT    48
  #define PIN_RIGHT   1
  #define PIN_SELECT  39
  #define PIN_HOME    40
  #define PIN_BUTTONA 41
  #define PIN_BUTTONB 42
#else
  // Real ESP32-S3 Handheld Hardware
  #define TFT_RST       -1   // Connected to ESP32 reset
  #define TFT_ROTATION  3    // Landscape orientation for physical display
  #define TFT_SPI_FREQ  40000000

  // Input Button Pins (Active-LOW)
  // Note: If using the PCB revision from HARDWARE_REVIEW.md, SELECT=39 and BUTTONB=42.
  #define PIN_UP      45
  #define PIN_DOWN    2
  #define PIN_LEFT    48
  #define PIN_RIGHT   1
  #define PIN_SELECT  16
  #define PIN_HOME    40
  #define PIN_BUTTONA 41
  #define PIN_BUTTONB 17
#endif

// Audio & Power pins (optional; define ENABLE_AUDIO to activate piezo output)
#define PIN_PIEZO   7
#define PIN_VOLUME  8
