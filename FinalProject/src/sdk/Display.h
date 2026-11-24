#pragma once

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include "Sprite.h"

namespace sdk {

/**
 * @brief 16-bit RGB565 offscreen canvas backed by PSRAM (with SRAM fallback).
 */
class PsramCanvas16 : public GFXcanvas16 {
public:
  PsramCanvas16(uint16_t w, uint16_t h);
  bool isReady() const { return buffer != nullptr; }
};

/**
 * @brief High-level display wrapper supporting both flicker-free framebuffer
 *        rendering and direct-to-TFT rendering.
 */
class Display {
public:
  explicit Display(Adafruit_ILI9341& tft);
  ~Display();

  void begin();

  // Enable/disable offscreen 320x240 framebuffer (allocated lazily on first enable)
  bool setBuffered(bool enabled);
  bool isBuffered() const { return buffered_ && canvas_ != nullptr && canvas_->isReady(); }

  int width() const { return tft_.width(); }
  int height() const { return tft_.height(); }

  // Active Adafruit_GFX target (canvas when buffered, TFT when direct)
  Adafruit_GFX& gfx();
  Adafruit_ILI9341& rawTft() { return tft_; }

  // High-level drawing helpers
  void clear(uint16_t color = Colors::Black);
  void drawSprite(const Sprite& sprite, int x, int y, int scale = 1, bool flipX = false);
  void drawText(const char* text, int x, int y, uint16_t color, uint8_t size = 1);
  void drawCenteredText(const char* text, int y, uint16_t color, uint8_t size = 1);
  void drawPanel(int x, int y, int w, int h, uint16_t bg, uint16_t border);

  // Flush the offscreen framebuffer to the physical ILI9341 display
  void present();
  void presentRect(int x, int y, int w, int h);

private:
  Adafruit_ILI9341& tft_;
  PsramCanvas16* canvas_ = nullptr;
  bool buffered_ = false;
  bool allocFailed_ = false;
};

} // namespace sdk
