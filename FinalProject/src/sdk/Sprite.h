#pragma once

#include <Arduino.h>

namespace sdk {

/**
 * @brief Reusable RGB565 sprite descriptor stored in PROGMEM.
 */
struct Sprite {
  const uint16_t* pixels = nullptr;
  uint16_t width = 0;
  uint16_t height = 0;
  uint16_t transparentColor = 0xF81F; // Default magenta key
  bool hasTransparency = true;

  bool isValid() const {
    return pixels != nullptr && width > 0 && height > 0;
  }
};

/**
 * @brief Common RGB565 color palette and RGB888 -> RGB565 converter.
 */
namespace Colors {
  constexpr uint16_t Black      = 0x0000;
  constexpr uint16_t White      = 0xFFFF;
  constexpr uint16_t Red        = 0xF800;
  constexpr uint16_t Green      = 0x07E0;
  constexpr uint16_t Blue       = 0x001F;
  constexpr uint16_t Yellow     = 0xFFE0;
  constexpr uint16_t Cyan       = 0x07FF;
  constexpr uint16_t Magenta    = 0xF81F;
  constexpr uint16_t Orange     = 0xFD20;
  constexpr uint16_t Purple     = 0x780F;
  constexpr uint16_t DarkGray   = 0x39E7;
  constexpr uint16_t MidGray    = 0x7BEF;
  constexpr uint16_t LightGray  = 0xBDF7;
  constexpr uint16_t PanelBg    = 0x18E3;
  constexpr uint16_t PanelLine  = 0x4208;
  constexpr uint16_t GrassGreen = 0x2624;
  constexpr uint16_t SkyBlue    = 0x5DDF;

  constexpr uint16_t rgb(uint8_t r, uint8_t g, uint8_t b) {
    return static_cast<uint16_t>(((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3));
  }
}

} // namespace sdk
