/**
 * @file main.cpp
 * @author Ethan Quigley (https://github.com/equigs8)
 * @author August (https://github.com/goosey999)
 * @brief Main OS shell for the ESP32-S3 Handheld Game Console.
 *
 * All games live under src/games/ and register themselves in src/games/GameList.h.
 */

#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <math.h>
#include "qrCode.h"

// Handheld Game SDK & Registered Games
#include "sdk/BoardConfig.h"
#include "sdk/Console.h"
#include "sdk/GameRegistry.h"
#include "games/GameList.h"

// ==============================================================================
// 1. DISPLAY, SDK RUNTIME & SYSTEM STATE
// ==============================================================================

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);
sdk::Console console(tft);

enum SystemState {
  STATE_NEW_MENU,
  STATE_SETTINGS,
  STATE_GAME
};
SystemState currentState = STATE_NEW_MENU;

// Shared UI Colors
#define BLACK           0x0000
#define WHITE           0xFFFF
#define RED             0xF800
#define YELLOW          0xFFE0
#define CURSOR_COLOR    0xF800
#define MAIN_MENU_COLOR BLACK

#define SERIAL_BAUD_RATE 9600

void resetSettings();
void settingsSelected();
void drawNewMenu();
void drawNewMenuCursor(int prevSelection, int newSelection);
void updateNewMenuCursorAnimation();

// ==============================================================================
// 2. GENERAL INPUT HANDLER (HOME / GLOBAL SHORTCUTS)
// ==============================================================================

void goHome() {
  if (console.hasActiveGame()) {
    console.exitGame();
  }
  currentState = STATE_NEW_MENU;
  drawNewMenu();
  delay(300); // Debounce
}

void handleGeneralInput() {
  static unsigned long lastMoveTime = 0;
  const unsigned long moveDelay = 200;
  unsigned long currentTime = millis();

  if (currentTime - lastMoveTime >= moveDelay) {
    bool moved = false;
    if (digitalRead(PIN_HOME) == LOW) {
      Serial.println("Home Button Pressed");
      moved = true;
      goHome();
    }
    if (digitalRead(PIN_BUTTONB) == LOW) {
      Serial.println("Button B Pressed");
      moved = true;
    }
    if (digitalRead(PIN_BUTTONA) == LOW) {
      Serial.println("Button A Pressed");
      moved = true;
    }
    if (digitalRead(PIN_UP) == LOW) {
      Serial.println("Button Up Pressed");
      moved = true;
    }
    if (digitalRead(PIN_DOWN) == LOW) {
      Serial.println("Button Down Pressed");
      moved = true;
    }
    if (digitalRead(PIN_LEFT) == LOW) {
      Serial.println("Button Left Pressed");
      moved = true;
    }
    if (digitalRead(PIN_RIGHT) == LOW) {
      Serial.println("Button Right Pressed");
      moved = true;
    }
    if (digitalRead(PIN_SELECT) == LOW) {
      Serial.println("Select Pressed");
      moved = true;
    }

    if (moved) {
      lastMoveTime = currentTime;
    }
  }
}

void settingsSelected() {
  currentState = STATE_SETTINGS;
  resetSettings();
}

// ==============================================================================
// 3. SETTINGS MENU
// ==============================================================================

int previousSettingsMenuSelection = 0;
int settingsMenuSelection = 0;
const int numSettingsMenu = 2;
const char* settingsMenu[2] = {"Volume", "Coming Soon..."};

void drawSettingsMenuCursor(int previousSelection, int currentSelection) {
  tft.fillRect(0, previousSelection * 40, tft.width(), 40, BLACK);
  tft.setCursor(0, previousSelection * 40);
  tft.setTextColor(WHITE);
  tft.setTextSize(2);
  tft.println(settingsMenu[previousSelection]);

  tft.fillRect(0, currentSelection * 40, tft.width(), 40, BLACK);
  tft.setCursor(0, currentSelection * 40);
  tft.setTextColor(YELLOW);
  tft.setTextSize(2);
  tft.println(settingsMenu[currentSelection]);
  tft.setTextColor(WHITE);
  previousSettingsMenuSelection = currentSelection;
}

void drawSettingsMenu(int previousSelection, int currentSelection) {
  (void)previousSelection;
  tft.fillScreen(BLACK);
  tft.setFont();
  tft.setTextSize(2);

  for (int i = 0; i < numSettingsMenu; i++) {
    tft.setCursor(0, i * 40);
    tft.setTextColor((i == currentSelection) ? YELLOW : WHITE);
    tft.println(settingsMenu[i]);
  }
  tft.setTextColor(WHITE);
  previousSettingsMenuSelection = currentSelection;
}

void handleSettingsInput() {
  static unsigned long lastMoveTime = 0;
  const unsigned long moveDelay = 200;
  unsigned long currentTime = millis();

  if (currentTime - lastMoveTime >= moveDelay) {
    bool moved = false;

    if (digitalRead(PIN_UP) == LOW) {
      settingsMenuSelection = max(0, settingsMenuSelection - 1);
      moved = true;
    } else if (digitalRead(PIN_DOWN) == LOW) {
      settingsMenuSelection = min(numSettingsMenu - 1, settingsMenuSelection + 1);
      moved = true;
    } else if (digitalRead(PIN_BUTTONB) == LOW) {
      goHome();
      return;
    }

    if (moved) {
      lastMoveTime = currentTime;
      drawSettingsMenuCursor(previousSettingsMenuSelection, settingsMenuSelection);
    }
  }
}

void resetSettings() {
  tft.fillScreen(BLACK);
  drawSettingsMenu(previousSettingsMenuSelection, settingsMenuSelection);
  drawSettingsMenuCursor(previousSettingsMenuSelection, settingsMenuSelection);
}

// ==============================================================================
// 4. NEW MAIN MENU (DYNAMIC GAME REGISTRY + ANIMATED CURSOR)
// ==============================================================================

int newMenuSelection = 0; // 0-5: 3x2 app grid, 6: Settings gear in top bar
int newMenuLastCol = 0;   // Remembers column when moving between top row and settings gear
int newMenuPage = 0;      // Supports >6 registered games across multiple 3x2 pages
const int newMenuAppX[6] = {36, 128, 220, 36, 128, 220};
const int newMenuAppY[6] = {53, 53, 53, 154, 154, 154};

static const unsigned char PROGMEM newMenuSettingsGearBits[] = {
  0x03,0xc0,0x12,0x48,0x2c,0x34,0x40,0x02,0x23,0xc4,0x24,0x24,0xc8,0x13,0x88,0x11,
  0x88,0x11,0xc8,0x13,0x24,0x24,0x23,0xc4,0x40,0x02,0x2c,0x34,0x12,0x48,0x03,0xc0
};

#define NEW_MENU_BG_COLOR         0xFFFF
#define NEW_MENU_TILE_BG_COLOR    0xBDF7
#define NEW_MENU_CURSOR_HIGHLIGHT 0xFBCA
#define NEW_MENU_GLIDE_MS         140UL
#define NEW_MENU_GLIDE_FRAME_MS   16UL
#define NEW_MENU_PULSE_PERIOD_MS  1200UL
#define NEW_MENU_PULSE_FRAME_MS   33UL
#define NEW_MENU_BRACKET_MIN_PAD  5
#define NEW_MENU_BRACKET_MAX_PAD  8
#define NEW_MENU_BRACKET_ARM      11

struct NewMenuRect {
  int x;
  int y;
  int w;
  int h;
};

struct NewMenuCursorAnim {
  bool active;
  unsigned long startTime;
  unsigned long lastFrame;
  unsigned long pulseStart;
  NewMenuRect startRect;
  NewMenuRect targetRect;
  NewMenuRect drawnRect;
  bool hasDrawnRect;
  int drawnBracketIndex;
  int drawnBracketPad;
};

NewMenuCursorAnim newMenuAnim = {
  false, 0, 0, 0,
  {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0},
  false, -1, -1
};

uint16_t blendRGB565(uint16_t c1, uint16_t c2, uint8_t alpha) {
  int r1 = (c1 >> 11) & 0x1F;
  int g1 = (c1 >> 5) & 0x3F;
  int b1 = c1 & 0x1F;

  int r2 = (c2 >> 11) & 0x1F;
  int g2 = (c2 >> 5) & 0x3F;
  int b2 = c2 & 0x1F;

  int r = r1 + ((r2 - r1) * (int)alpha) / 255;
  int g = g1 + ((g2 - g1) * (int)alpha) / 255;
  int b = b1 + ((b2 - b1) * (int)alpha) / 255;

  return (uint16_t)((r << 11) | (g << 5) | b);
}

NewMenuRect getNewMenuTargetRect(int index) {
  if (index >= 0 && index < 6) {
    return {newMenuAppX[index] - 2, newMenuAppY[index] - 2, 68, 68};
  }
  return {1, 0, 20, 20}; // Settings gear inside top bar
}

int getNewMenuTotalPages() {
  int count = sdk::GameRegistry::count();
  return max(1, (count + 5) / 6);
}

sdk::Game* getNewMenuGameAtSlot(int slot) {
  if (slot < 0 || slot >= 6) return nullptr;
  return sdk::GameRegistry::get(newMenuPage * 6 + slot);
}

void drawNewMenuTopTitle(int selection) {
  tft.fillRect(26, 0, 240, 20, NEW_MENU_TILE_BG_COLOR);

  const char* label = "";
  if (selection == 6) {
    label = "Settings";
  } else {
    sdk::Game* game = getNewMenuGameAtSlot(selection);
    label = game ? game->title() : "Empty Slot";
  }

  char headerBuf[32];
  int totalPages = getNewMenuTotalPages();
  if (totalPages > 1 && selection != 6) {
    snprintf(headerBuf, sizeof(headerBuf), "%s (%d/%d)", label, newMenuPage + 1, totalPages);
  } else {
    snprintf(headerBuf, sizeof(headerBuf), "%s", label);
  }

  int textWidth = (int)strlen(headerBuf) * 6;
  int xPos = max(28, (320 - textWidth) / 2);
  tft.setFont();
  tft.setTextColor(0x0000);
  tft.setTextSize(1);
  tft.setCursor(xPos, 6);
  tft.print(headerBuf);
}

uint16_t getNewMenuPixelColor(int px, int py) {
  if (py < 20) {
    if (px >= 3 && px < 19 && py >= 2 && py < 18) {
      int gx = px - 3;
      int gy = py - 2;
      uint8_t byteVal = pgm_read_byte(&newMenuSettingsGearBits[gy * 2 + (gx >> 3)]);
      if (byteVal & (0x80 >> (gx & 7))) {
        return 0x0000;
      }
    }
    return NEW_MENU_TILE_BG_COLOR;
  }

  for (int i = 0; i < 6; i++) {
    int tx = newMenuAppX[i];
    int ty = newMenuAppY[i];
    if (px >= tx && px < tx + 64 && py >= ty && py < ty + 64) {
      sdk::Game* game = getNewMenuGameAtSlot(i);
      if (game) {
        sdk::Sprite icon = game->icon();
        if (icon.isValid()) {
          int sx = ((px - tx) * icon.width) / 64;
          int sy = ((py - ty) * icon.height) / 64;
          uint16_t c = pgm_read_word(&icon.pixels[sy * icon.width + sx]);
          if (!icon.hasTransparency || c != icon.transparentColor) {
            return c;
          }
        }
      }
      return NEW_MENU_TILE_BG_COLOR;
    }
  }

  return NEW_MENU_BG_COLOR;
}

void restoreNewMenuHLine(int x, int y, int w) {
  if (y < 0 || y >= 240 || w <= 0) return;
  int xStart = max(0, x);
  int xEnd = min(320, x + w);
  if (xStart >= xEnd) return;

  int runStart = xStart;
  uint16_t runColor = getNewMenuPixelColor(xStart, y);
  for (int px = xStart + 1; px < xEnd; px++) {
    uint16_t c = getNewMenuPixelColor(px, y);
    if (c != runColor) {
      tft.writeFastHLine(runStart, y, px - runStart, runColor);
      runStart = px;
      runColor = c;
    }
  }
  tft.writeFastHLine(runStart, y, xEnd - runStart, runColor);
}

void restoreNewMenuVLine(int x, int y, int h) {
  if (x < 0 || x >= 320 || h <= 0) return;
  int yStart = max(0, y);
  int yEnd = min(240, y + h);
  if (yStart >= yEnd) return;

  int runStart = yStart;
  uint16_t runColor = getNewMenuPixelColor(x, yStart);
  for (int py = yStart + 1; py < yEnd; py++) {
    uint16_t c = getNewMenuPixelColor(x, py);
    if (c != runColor) {
      tft.writeFastVLine(x, runStart, py - runStart, runColor);
      runStart = py;
      runColor = c;
    }
  }
  tft.writeFastVLine(x, runStart, yEnd - runStart, runColor);
}

void restoreNewMenuBoxOutlineRect(const NewMenuRect &r) {
  if (r.w < 4 || r.h < 4) return;
  tft.startWrite();
  restoreNewMenuHLine(r.x, r.y, r.w);
  restoreNewMenuHLine(r.x, r.y + 1, r.w);
  restoreNewMenuHLine(r.x, r.y + r.h - 2, r.w);
  restoreNewMenuHLine(r.x, r.y + r.h - 1, r.w);
  restoreNewMenuVLine(r.x, r.y + 2, r.h - 4);
  restoreNewMenuVLine(r.x + 1, r.y + 2, r.h - 4);
  restoreNewMenuVLine(r.x + r.w - 2, r.y + 2, r.h - 4);
  restoreNewMenuVLine(r.x + r.w - 1, r.y + 2, r.h - 4);
  tft.endWrite();
}

void drawNewMenuBoxOutlineRect(const NewMenuRect &r, uint16_t color) {
  if (r.w < 4 || r.h < 4) return;
  tft.startWrite();
  tft.writeFastHLine(r.x, r.y, r.w, color);
  tft.writeFastHLine(r.x, r.y + 1, r.w, color);
  tft.writeFastHLine(r.x, r.y + r.h - 2, r.w, color);
  tft.writeFastHLine(r.x, r.y + r.h - 1, r.w, color);
  tft.writeFastVLine(r.x, r.y + 2, r.h - 4, color);
  tft.writeFastVLine(r.x + 1, r.y + 2, r.h - 4, color);
  tft.writeFastVLine(r.x + r.w - 2, r.y + 2, r.h - 4, color);
  tft.writeFastVLine(r.x + r.w - 1, r.y + 2, r.h - 4, color);
  tft.endWrite();
}

void drawNewMenuCornerBrackets(int index, int pad, uint16_t color) {
  if (index < 0 || index >= 6) return;
  int bx = newMenuAppX[index] - pad;
  int by = newMenuAppY[index] - pad;
  int bw = 64 + pad * 2;
  int bh = 64 + pad * 2;
  const int arm = NEW_MENU_BRACKET_ARM;

  tft.startWrite();
  tft.writeFillRect(bx, by, arm, 2, color);
  tft.writeFillRect(bx, by + 2, 2, arm - 2, color);
  tft.writeFillRect(bx + bw - arm, by, arm, 2, color);
  tft.writeFillRect(bx + bw - 2, by + 2, 2, arm - 2, color);
  tft.writeFillRect(bx, by + bh - 2, arm, 2, color);
  tft.writeFillRect(bx, by + bh - arm, 2, arm - 2, color);
  tft.writeFillRect(bx + bw - arm, by + bh - 2, arm, 2, color);
  tft.writeFillRect(bx + bw - 2, by + bh - arm, 2, arm - 2, color);
  tft.endWrite();
}

void drawNewMenuSettingsCorners(uint16_t color) {
  tft.startWrite();
  tft.writeFastHLine(1, 0, 3, color);
  tft.writeFastVLine(1, 0, 3, color);
  tft.writeFastHLine(18, 0, 3, color);
  tft.writeFastVLine(20, 0, 3, color);
  tft.writeFastHLine(1, 19, 3, color);
  tft.writeFastVLine(1, 17, 3, color);
  tft.writeFastHLine(18, 19, 3, color);
  tft.writeFastVLine(20, 17, 3, color);
  tft.endWrite();
}

void drawNewMenuCursor(int prevSelection, int newSelection) {
  unsigned long now = millis();

  if (prevSelection < 0) {
    newMenuAnim.active = false;
    newMenuAnim.targetRect = getNewMenuTargetRect(newSelection);
    newMenuAnim.startRect = newMenuAnim.targetRect;
    newMenuAnim.drawnRect = newMenuAnim.targetRect;
    newMenuAnim.hasDrawnRect = true;
    newMenuAnim.pulseStart = now;
    newMenuAnim.lastFrame = now;
    newMenuAnim.drawnBracketIndex = -1;
    newMenuAnim.drawnBracketPad = -1;

    drawNewMenuBoxOutlineRect(newMenuAnim.drawnRect, CURSOR_COLOR);
    if (newSelection >= 0 && newSelection < 6) {
      drawNewMenuCornerBrackets(newSelection, NEW_MENU_BRACKET_MIN_PAD, CURSOR_COLOR);
      newMenuAnim.drawnBracketIndex = newSelection;
      newMenuAnim.drawnBracketPad = NEW_MENU_BRACKET_MIN_PAD;
    }
    return;
  }

  if (prevSelection == newSelection) return;

  if (newMenuAnim.drawnBracketIndex != -1) {
    drawNewMenuCornerBrackets(newMenuAnim.drawnBracketIndex, newMenuAnim.drawnBracketPad, NEW_MENU_BG_COLOR);
    newMenuAnim.drawnBracketIndex = -1;
    newMenuAnim.drawnBracketPad = -1;
  }

  newMenuAnim.startRect = newMenuAnim.hasDrawnRect ? newMenuAnim.drawnRect : getNewMenuTargetRect(prevSelection);
  if (!newMenuAnim.hasDrawnRect) {
    newMenuAnim.drawnRect = newMenuAnim.startRect;
    newMenuAnim.hasDrawnRect = true;
  }
  newMenuAnim.targetRect = getNewMenuTargetRect(newSelection);
  newMenuAnim.startTime = now;
  newMenuAnim.lastFrame = 0;
  newMenuAnim.active = true;
}

void updateNewMenuCursorAnimation() {
  if (!newMenuAnim.hasDrawnRect) return;
  unsigned long now = millis();

  if (newMenuAnim.active) {
    if (newMenuAnim.lastFrame != 0 && (now - newMenuAnim.lastFrame) < NEW_MENU_GLIDE_FRAME_MS) {
      return;
    }

    unsigned long elapsed = now - newMenuAnim.startTime;
    float t = (elapsed >= NEW_MENU_GLIDE_MS) ? 1.0f : (float)elapsed / (float)NEW_MENU_GLIDE_MS;
    float inv = 1.0f - t;
    float ease = 1.0f - (inv * inv * inv);

    NewMenuRect nextRect;
    if (t >= 1.0f) {
      nextRect = newMenuAnim.targetRect;
    } else {
      nextRect.x = newMenuAnim.startRect.x + (int)lroundf((newMenuAnim.targetRect.x - newMenuAnim.startRect.x) * ease);
      nextRect.y = newMenuAnim.startRect.y + (int)lroundf((newMenuAnim.targetRect.y - newMenuAnim.startRect.y) * ease);
      nextRect.w = newMenuAnim.startRect.w + (int)lroundf((newMenuAnim.targetRect.w - newMenuAnim.startRect.w) * ease);
      nextRect.h = newMenuAnim.startRect.h + (int)lroundf((newMenuAnim.targetRect.h - newMenuAnim.startRect.h) * ease);
    }

    bool rectChanged = (nextRect.x != newMenuAnim.drawnRect.x ||
                        nextRect.y != newMenuAnim.drawnRect.y ||
                        nextRect.w != newMenuAnim.drawnRect.w ||
                        nextRect.h != newMenuAnim.drawnRect.h);

    if (rectChanged) {
      restoreNewMenuBoxOutlineRect(newMenuAnim.drawnRect);
      uint16_t glideColor = blendRGB565(CURSOR_COLOR, NEW_MENU_CURSOR_HIGHLIGHT, (uint8_t)((1.0f - t) * 140.0f));
      drawNewMenuBoxOutlineRect(nextRect, glideColor);
      newMenuAnim.drawnRect = nextRect;
    }

    if (t >= 1.0f) {
      newMenuAnim.active = false;
      newMenuAnim.pulseStart = now;
      drawNewMenuBoxOutlineRect(newMenuAnim.drawnRect, CURSOR_COLOR);
      if (newMenuSelection >= 0 && newMenuSelection < 6) {
        drawNewMenuCornerBrackets(newMenuSelection, NEW_MENU_BRACKET_MIN_PAD, CURSOR_COLOR);
        newMenuAnim.drawnBracketIndex = newMenuSelection;
        newMenuAnim.drawnBracketPad = NEW_MENU_BRACKET_MIN_PAD;
      }
      drawNewMenuTopTitle(newMenuSelection);
    }

    newMenuAnim.lastFrame = now;
    return;
  }

  if ((now - newMenuAnim.lastFrame) < NEW_MENU_PULSE_FRAME_MS) {
    return;
  }
  newMenuAnim.lastFrame = now;

  float phase = (float)((now - newMenuAnim.pulseStart) % NEW_MENU_PULSE_PERIOD_MS) / (float)NEW_MENU_PULSE_PERIOD_MS;
  float wave = 0.5f - 0.5f * cosf(phase * 6.2831853f);

  uint16_t frameColor = blendRGB565(CURSOR_COLOR, NEW_MENU_CURSOR_HIGHLIGHT, (uint8_t)(wave * 200.0f));
  drawNewMenuBoxOutlineRect(newMenuAnim.drawnRect, frameColor);

  if (newMenuSelection >= 0 && newMenuSelection < 6) {
    int targetPad = NEW_MENU_BRACKET_MIN_PAD + (int)lroundf(wave * (float)(NEW_MENU_BRACKET_MAX_PAD - NEW_MENU_BRACKET_MIN_PAD));
    uint16_t bracketColor = blendRGB565(CURSOR_COLOR, NEW_MENU_CURSOR_HIGHLIGHT, (uint8_t)(wave * 255.0f));

    if (newMenuAnim.drawnBracketIndex != newMenuSelection || newMenuAnim.drawnBracketPad != targetPad) {
      if (newMenuAnim.drawnBracketIndex != -1) {
        drawNewMenuCornerBrackets(newMenuAnim.drawnBracketIndex, newMenuAnim.drawnBracketPad, NEW_MENU_BG_COLOR);
      }
      drawNewMenuCornerBrackets(newMenuSelection, targetPad, bracketColor);
      newMenuAnim.drawnBracketIndex = newMenuSelection;
      newMenuAnim.drawnBracketPad = targetPad;
    } else {
      drawNewMenuCornerBrackets(newMenuSelection, targetPad, bracketColor);
    }
  } else if (newMenuSelection == 6) {
    uint16_t cornerColor = blendRGB565(frameColor, WHITE, (uint8_t)(wave * 150.0f));
    drawNewMenuSettingsCorners(cornerColor);
  }
}

void playNewMenuSelectAnimation(int selection) {
  if (newMenuAnim.active) {
    restoreNewMenuBoxOutlineRect(newMenuAnim.drawnRect);
    newMenuAnim.drawnRect = newMenuAnim.targetRect;
    newMenuAnim.active = false;
  }
  if (newMenuAnim.drawnBracketIndex != -1) {
    drawNewMenuCornerBrackets(newMenuAnim.drawnBracketIndex, newMenuAnim.drawnBracketPad, NEW_MENU_BG_COLOR);
    newMenuAnim.drawnBracketIndex = -1;
    newMenuAnim.drawnBracketPad = -1;
  }

  uint16_t flashColor = blendRGB565(CURSOR_COLOR, WHITE, 190);
  if (selection >= 0 && selection < 6) {
    drawNewMenuBoxOutlineRect(newMenuAnim.drawnRect, NEW_MENU_CURSOR_HIGHLIGHT);
    drawNewMenuCornerBrackets(selection, NEW_MENU_BRACKET_MIN_PAD - 1, CURSOR_COLOR);
    delay(35);
    drawNewMenuCornerBrackets(selection, NEW_MENU_BRACKET_MIN_PAD - 1, NEW_MENU_BG_COLOR);
    drawNewMenuBoxOutlineRect(newMenuAnim.drawnRect, flashColor);
    delay(35);
    drawNewMenuBoxOutlineRect(newMenuAnim.drawnRect, CURSOR_COLOR);
    delay(25);
  } else if (selection == 6) {
    drawNewMenuBoxOutlineRect(newMenuAnim.drawnRect, flashColor);
    delay(45);
    drawNewMenuBoxOutlineRect(newMenuAnim.drawnRect, CURSOR_COLOR);
    delay(35);
  }
}

void drawNewMenu() {
  tft.fillScreen(0x0);
  tft.fillRect(0, 20, 320, 220, NEW_MENU_BG_COLOR);

  // Dynamically draw the 3x2 app grid from sdk::GameRegistry
  for (int i = 0; i < 6; i++) {
    int tx = newMenuAppX[i];
    int ty = newMenuAppY[i];
    tft.fillRect(tx, ty, 64, 64, NEW_MENU_TILE_BG_COLOR);

    sdk::Game* game = getNewMenuGameAtSlot(i);
    if (game) {
      sdk::Sprite icon = game->icon();
      if (icon.isValid()) {
        int scale = max(1, 64 / (int)icon.width);
        console.display.drawSprite(icon, tx, ty, scale);
      }
    }
  }

  // Top Bar
  tft.fillRect(0, 0, 320, 20, NEW_MENU_TILE_BG_COLOR);
  tft.drawBitmap(3, 2, newMenuSettingsGearBits, 16, 16, 0x0);
  drawNewMenuTopTitle(newMenuSelection);

  tft.setTextColor(0x0);
  tft.setTextSize(1);
  tft.setCursor(285, 6);
  tft.print("12:45");

  drawNewMenuCursor(-1, newMenuSelection);
}

void handleNewMenuInput() {
  static unsigned long lastMoveTime = 0;
  const unsigned long moveDelay = 200;
  unsigned long currentTime = millis();
  int prevSelection = newMenuSelection;

  if (currentTime - lastMoveTime >= moveDelay) {
    bool moved = false;
    bool pageChanged = false;
    int totalPages = getNewMenuTotalPages();

    if (newMenuSelection == 6) {
      if (digitalRead(PIN_DOWN) == LOW) {
        newMenuSelection = newMenuLastCol;
        moved = true;
      }
    } else {
      int row = newMenuSelection / 3;
      int col = newMenuSelection % 3;

      if (digitalRead(PIN_UP) == LOW) {
        if (row == 1) {
          newMenuSelection -= 3;
        } else {
          newMenuLastCol = col;
          newMenuSelection = 6;
        }
        moved = true;
      } else if (digitalRead(PIN_DOWN) == LOW) {
        if (row == 0) {
          newMenuSelection += 3;
          moved = true;
        }
      } else if (digitalRead(PIN_LEFT) == LOW) {
        if (col > 0) {
          newMenuSelection -= 1;
          moved = true;
        } else if (totalPages > 1 && newMenuPage > 0) {
          newMenuPage -= 1;
          newMenuSelection = row * 3 + 2;
          pageChanged = true;
        }
      } else if (digitalRead(PIN_RIGHT) == LOW) {
        if (col < 2) {
          newMenuSelection += 1;
          moved = true;
        } else if (totalPages > 1 && newMenuPage + 1 < totalPages) {
          newMenuPage += 1;
          newMenuSelection = row * 3;
          pageChanged = true;
        }
      }
    }

    if (pageChanged) {
      lastMoveTime = currentTime;
      drawNewMenu();
    } else if (moved) {
      lastMoveTime = currentTime;
      drawNewMenuTopTitle(newMenuSelection);
      drawNewMenuCursor(prevSelection, newMenuSelection);
    }
  }

  updateNewMenuCursorAnimation();

  if (digitalRead(PIN_BUTTONA) == LOW) {
    if (newMenuSelection == 6) {
      playNewMenuSelectAnimation(6);
      settingsSelected();
      delay(200);
    } else {
      sdk::Game* selectedGame = getNewMenuGameAtSlot(newMenuSelection);
      if (selectedGame) {
        playNewMenuSelectAnimation(newMenuSelection);
        currentState = STATE_GAME;
        console.launchGame(selectedGame);
      }
      delay(200);
    }
  }
}

void drawQRCode() {
  tft.drawBitmap(0, 0, QR_CODE_BITS, 135, 135, WHITE, BLACK);
}

// ==============================================================================
// 5. ARDUINO SETUP & LOOP
// ==============================================================================

void setup() {
  Serial.begin(SERIAL_BAUD_RATE);
  delay(500);

  SPI.begin(TFT_SCK, TFT_MISO, TFT_MOSI, TFT_CS);
  tft.begin(TFT_SPI_FREQ);
  delay(100);
  tft.setRotation(TFT_ROTATION);

  console.begin();
  registerAllGames();

  tft.fillScreen(MAIN_MENU_COLOR);
  drawNewMenu();

  Serial.println("Starting up...");
}

void loop() {
  handleGeneralInput();

  switch (currentState) {
    case STATE_NEW_MENU:
      handleNewMenuInput();
      break;
    case STATE_SETTINGS:
      handleSettingsInput();
      break;
    case STATE_GAME:
      console.tick();
      if (console.exitRequested()) {
        goHome();
      }
      break;
  }
}
