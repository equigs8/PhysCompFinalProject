# Handheld Game Developer Guide

Welcome! This guide shows you how to build, test, and add your own game to the **ESP32-S3 Handheld Console** in under 5 minutes—without touching SPI drivers, pinouts, button debouncing, or the main menu code.

---

## 1. Build & Run (Wokwi Simulator vs. Real Hardware)

The project uses two preconfigured PlatformIO environments in `FinalProject/platformio.ini` so you never have to manually edit pins or screen rotation:

| Environment | Target | Display Rotation | Command |
| :--- | :--- | :--- | :--- |
| **`wokwi`** *(default)* | Wokwi Simulator (`diagram.json`) | `1` (320×240) | `pio run -d FinalProject -e wokwi` |
| **`hardware`** | Real ESP32-S3 Handheld (PSRAM enabled) | `3` (320×240) | `pio run -d FinalProject -e hardware -t upload` |

> **Wokwi Keyboard Controls (`diagram.json`):**
> `W / A / S / D` = D-Pad · `G` = Button A · `H` = Button B · `Enter` = Select · `Backspace` = Home (Exit to Main Menu)

---

## 2. Create Your First Game in 5 Minutes

Every game lives in its own self-contained folder under `FinalProject/src/games/`:
```
src/games/my_game/
├── MyGame.h         (and optional MyGame.cpp)
└── assets/
    ├── icon.h       (16x16 Main Menu icon)
    └── hero.h       (optional sprites)
```

### Step 1 — Create Your Game Folder
Duplicate [`src/games/template/`](../src/games/template/) to `src/games/my_game/` (or generate one using [`tools/sprite-studio.html`](../tools/sprite-studio.html)), rename `TemplateGame.h` → `MyGame.h`, and rename the class to `MyGame`:

```cpp
#pragma once
#include "../../sdk/Game.h"
#include "../../sdk/Console.h"
#include "assets/icon.h"

class MyGame : public sdk::Game {
public:
  const char* title() const override { return "My Game"; }
  sdk::Sprite icon() const override { return TEMPLATE_ICON; }

  void onEnter(sdk::Console& console) override {
    x_ = 150.0f;
    y_ = 110.0f;
  }

  void onUpdate(sdk::Console& console, float dt) override {
    // Smooth D-pad movement (120 pixels/sec)
    x_ += console.input.axisX() * 120.0f * dt;
    y_ += console.input.axisY() * 120.0f * dt;

    if (console.input.pressed(sdk::BTN_A)) {
      console.audio.playSelect();
    }
  }

  void onRender(sdk::Console& console) override {
    console.display.clear(sdk::Colors::PanelBg);
    console.display.drawCenteredText("HELLO HANDHELD!", 12, sdk::Colors::Yellow, 2);
    console.display.gfx().fillRoundRect((int)x_, (int)y_, 20, 20, 4, sdk::Colors::Cyan);
  }

private:
  float x_ = 150.0f, y_ = 110.0f;
};
```

### Step 2 — Build & Play! (Auto-Discovered!)
You **do not** need to edit `GameList.h` or `main.cpp`! Before every build, [`scripts/discover_games.py`](../scripts/discover_games.py) scans `src/games/*/` and automatically adds/removes every game class inheriting from `sdk::Game`:

- **Add a game:** Drop its folder into `src/games/<my_game>/` and build → it appears in the Main Menu automatically.
- **Remove a game:** Delete its folder (or drop an empty `.disabled` file inside it) and build → it disappears from the Main Menu automatically.
- *(The `src/games/template/` folder and any folder starting with `_` are automatically ignored.)*

Build with `pio run -d FinalProject -e wokwi` (or `-e hardware -t upload`). Your game's 16×16 icon and title automatically appear in the Main Menu grid (with automatic paging once you exceed 6 games!). Pressing **HOME** cleanly exits back to the menu.

For full playable references, inspect [`src/games/snake/SnakeGame.h`](../src/games/snake/SnakeGame.h) and [`src/games/star_blaster/StarBlasterGame.h`](../src/games/star_blaster/StarBlasterGame.h).

---

## 3. SDK API Quick Reference

Every game receives `sdk::Console& console` in `onEnter`, `onUpdate`, `onRender`, and `onExit`.

### Display (`console.display`)
By default (`usesFramebuffer() == true`), every `onRender()` draws into a **320×240 RGB565 offscreen canvas** (allocated in PSRAM on real hardware, SRAM on Wokwi) and is blitted to the screen automatically with **zero flicker**.

| Method | Description |
| :--- | :--- |
| `console.display.clear(color)` | Fill the screen with an RGB565 color |
| `console.display.gfx()` | Returns `Adafruit_GFX&` (`fillRect`, `drawCircle`, `fillRoundRect`, `drawTriangle`, `drawLine`, etc.) |
| `console.display.drawSprite(sprite, x, y, scale, flipX)` | Draw an `sdk::Sprite` with optional integer scaling & horizontal flip |
| `console.display.drawText(str, x, y, color, size)` | Draw text at `(x, y)` |
| `console.display.drawCenteredText(str, y, color, size)` | Draw horizontally centered text |
| `console.display.drawPanel(x, y, w, h, bg, border)` | Draw a bordered UI card/HUD bar |
| `console.display.width()` / `height()` | Screen dimensions (`320` × `240`) |
| `sdk::Colors::rgb(r, g, b)` | Convert 0–255 RGB to RGB565 (plus presets: `Black`, `White`, `Red`, `Green`, `Blue`, `Yellow`, `Cyan`, `Orange`, `PanelBg`, etc.) |

### Input (`console.input`)
Buttons: `sdk::BTN_UP`, `BTN_DOWN`, `BTN_LEFT`, `BTN_RIGHT`, `BTN_A`, `BTN_B`, `BTN_SELECT`, `BTN_HOME`.

| Method | Description |
| :--- | :--- |
| `console.input.pressed(btn)` | `true` only on the single frame the button is pressed (debounced) |
| `console.input.held(btn)` | `true` continuously while held down |
| `console.input.released(btn)` | `true` on the single frame released |
| `console.input.repeated(btn, delayMs, repeatMs)` | Fires on press, then repeats while held (ideal for cursors/menus) |
| `console.input.axisX()` / `axisY()` | Returns `-1`, `0`, or `+1` for D-pad movement |

### Audio (`console.audio`)
Non-blocking piezo tone helper (never freezes your game loop):
- `console.audio.tone(freqHz, durationMs)` / `stop()`
- Presets: `playSelect()`, `playMove()`, `playPickup()`, `playHit()`, `playWin()`, `playLose()`
- *Note: Add `-DENABLE_AUDIO=1` to `build_flags` in `platformio.ini` once your piezo flyback diode/driver is installed.*

---

## 4. Creating Sprites & 16×16 Menu Icons

Never convert pixel arrays by hand! You have two tools:

### Option A — Browser Sprite Studio & Game Scaffolder (Zero Install)
Double-click [`FinalProject/tools/sprite-studio.html`](../tools/sprite-studio.html) to open it in any web browser:
- **Sprite & Icon Editor**: Drag-and-drop any PNG/JPG/GIF or paint pixel art directly in the built-in grid, preview at 1×/4× scale, and download a ready-to-include `.h` header.
- **New Game Scaffolder**: Enter a game title & class name, draw or drop a 16×16 icon, and download a complete compiling `MyGame.h` file.

### Option B — Command-Line Converter (`convert_image.py`)
```bash
# Generate a 16x16 Main Menu icon inside your game's assets/ folder
python FinalProject/convert_image.py icon.png --icon --name MY_ICON -o FinalProject/src/games/my_game/assets/icon.h

# Generate a custom-sized game sprite (e.g. 24x24)
python FinalProject/convert_image.py hero.png --sdk --name HERO_SPRITE -W 24 -H 24 -o FinalProject/src/games/my_game/assets/hero.h
```

---

## 5. Best Practices for Smooth Handheld Games

1. **Never use `delay()` inside `onUpdate()` or `onRender()`** — accumulate `dt` (seconds) in a timer instead (see `stepTimer_` in `SnakeGame.h`) so inputs and `HOME` stay instant.
2. **Store sprites in `PROGMEM`** — both `convert_image.py` and `sprite-studio.html` do this automatically so large art doesn't consume SRAM.
3. **Keep game state inside your class** — avoid global variables so multiple games never collide.
4. **Direct TFT mode (optional)** — if you're building a static puzzle/board game that only redraws one tile on button press, override `bool usesFramebuffer() const override { return false; }` and draw via `console.display.gfx()` or `console.display.rawTft()`.
