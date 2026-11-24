#include "Display.h"
#include "BoardConfig.h"
#include <string.h>

namespace sdk {

PsramCanvas16::PsramCanvas16(uint16_t w, uint16_t h) : GFXcanvas16(w, h, false) {
  size_t bytes = static_cast<size_t>(w) * static_cast<size_t>(h) * sizeof(uint16_t);
#if defined(BOARD_HAS_PSRAM)
  if (psramFound()) {
    buffer = static_cast<uint16_t*>(ps_malloc(bytes));
  }
#endif
  if (!buffer) {
    buffer = static_cast<uint16_t*>(malloc(bytes));
  }
  if (buffer) {
    buffer_owned = true;
    memset(buffer, 0, bytes);
  }
}

Display::Display(Adafruit_ILI9341& tft) : tft_(tft) {}

Display::~Display() {
  if (canvas_) {
    delete canvas_;
    canvas_ = nullptr;
  }
}

void Display::begin() {
  // TFT SPI & rotation are initialized in setup() using BoardConfig.h
}

bool Display::setBuffered(bool enabled) {
  if (!enabled) {
    buffered_ = false;
    return true;
  }
  if (!canvas_ && !allocFailed_) {
    canvas_ = new PsramCanvas16(SCREEN_WIDTH, SCREEN_HEIGHT);
    if (!canvas_ || !canvas_->isReady()) {
      delete canvas_;
      canvas_ = nullptr;
      allocFailed_ = true;
      buffered_ = false;
      Serial.println("[SDK] Framebuffer allocation failed; falling back to direct TFT.");
      return false;
    }
  }
  buffered_ = (canvas_ != nullptr && canvas_->isReady());
  return buffered_;
}

Adafruit_GFX& Display::gfx() {
  if (isBuffered()) {
    return *canvas_;
  }
  return tft_;
}

void Display::clear(uint16_t color) {
  gfx().fillScreen(color);
}

void Display::drawSprite(const Sprite& sprite, int x, int y, int scale, bool flipX) {
  if (!sprite.isValid()) return;
  if (scale < 1) scale = 1;

  const int w = sprite.width;
  const int h = sprite.height;

  if (isBuffered()) {
    uint16_t* buf = canvas_->getBuffer();
    const int cw = canvas_->width();
    const int ch = canvas_->height();

    for (int j = 0; j < h; ++j) {
      int dstY0 = y + j * scale;
      if (dstY0 + scale <= 0 || dstY0 >= ch) continue;

      for (int i = 0; i < w; ++i) {
        int srcX = flipX ? (w - 1 - i) : i;
        uint16_t color = pgm_read_word(&sprite.pixels[j * w + srcX]);
        if (sprite.hasTransparency && color == sprite.transparentColor) continue;

        int dstX0 = x + i * scale;
        if (dstX0 + scale <= 0 || dstX0 >= cw) continue;

        if (scale == 1) {
          buf[dstY0 * cw + dstX0] = color;
        } else {
          int yStart = max(0, dstY0);
          int yEnd = min(ch, dstY0 + scale);
          int xStart = max(0, dstX0);
          int xEnd = min(cw, dstX0 + scale);
          for (int py = yStart; py < yEnd; ++py) {
            uint16_t* row = &buf[py * cw];
            for (int px = xStart; px < xEnd; ++px) {
              row[px] = color;
            }
          }
        }
      }
    }
    return;
  }

  // Direct TFT path: single SPI transaction for fast flicker-minimal sprite blits
  tft_.startWrite();
  for (int j = 0; j < h; ++j) {
    for (int i = 0; i < w; ++i) {
      int srcX = flipX ? (w - 1 - i) : i;
      uint16_t color = pgm_read_word(&sprite.pixels[j * w + srcX]);
      if (sprite.hasTransparency && color == sprite.transparentColor) continue;

      if (scale == 1) {
        tft_.writePixel(x + i, y + j, color);
      } else {
        tft_.writeFillRect(x + i * scale, y + j * scale, scale, scale, color);
      }
    }
  }
  tft_.endWrite();
}

void Display::drawText(const char* text, int x, int y, uint16_t color, uint8_t size) {
  if (!text) return;
  Adafruit_GFX& g = gfx();
  g.setFont();
  g.setTextSize(size);
  g.setTextColor(color);
  g.setCursor(x, y);
  g.print(text);
}

void Display::drawCenteredText(const char* text, int y, uint16_t color, uint8_t size) {
  if (!text) return;
  int len = static_cast<int>(strlen(text));
  int textWidth = len * 6 * size;
  int x = max(0, (width() - textWidth) / 2);
  drawText(text, x, y, color, size);
}

void Display::drawPanel(int x, int y, int w, int h, uint16_t bg, uint16_t border) {
  Adafruit_GFX& g = gfx();
  g.fillRect(x, y, w, h, bg);
  g.drawRect(x, y, w, h, border);
}

void Display::present() {
  if (!isBuffered()) return;
  tft_.drawRGBBitmap(0, 0, canvas_->getBuffer(), canvas_->width(), canvas_->height());
}

void Display::presentRect(int x, int y, int w, int h) {
  if (!isBuffered()) return;
  const int cw = canvas_->width();
  const int ch = canvas_->height();

  int x0 = max(0, x);
  int y0 = max(0, y);
  int x1 = min(cw, x + w);
  int y1 = min(ch, y + h);
  int rw = x1 - x0;
  int rh = y1 - y0;
  if (rw <= 0 || rh <= 0) return;

  uint16_t* buf = canvas_->getBuffer();
  tft_.startWrite();
  tft_.setAddrWindow(x0, y0, rw, rh);
  for (int row = y0; row < y1; ++row) {
    tft_.writePixels(&buf[row * cw + x0], rw, true, false);
  }
  tft_.endWrite();
}

} // namespace sdk
