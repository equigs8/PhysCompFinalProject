#pragma once

#include <Arduino.h>
#include "Sprite.h"

namespace sdk {

class Console;

/**
 * @brief Base class for all handheld games.
 *
 * To create a new game:
 *   1. Subclass sdk::Game in src/games/MyGame.h
 *   2. Register it in src/games/GameList.h
 * The main menu automatically displays its icon, title, and launches it.
 */
class Game {
public:
  virtual ~Game() = default;

  // Displayed in the main menu top bar when highlighted
  virtual const char* title() const = 0;

  // 16x16 RGB565 icon displayed in the 3x2 main menu grid (scaled 4x to 64x64)
  virtual Sprite icon() const = 0;

  // Return true (default) for automatic flicker-free 320x240 framebuffer rendering.
  // Return false if your game draws directly to the TFT via console.display.rawTft().
  virtual bool usesFramebuffer() const { return true; }

  // Target render interval in ms for framebuffer games (default: 33ms = ~30 FPS)
  virtual uint16_t renderIntervalMs() const { return 33; }

  // Called once when the game is launched from the main menu
  virtual void onEnter(Console& console) = 0;

  // Called every loop iteration with elapsed seconds since last update
  virtual void onUpdate(Console& console, float dt) = 0;

  // Called at renderIntervalMs() (or every frame for direct games) to draw
  virtual void onRender(Console& console) = 0;

  // Called once when the player presses HOME to leave the game
  virtual void onExit(Console& console) { (void)console; }
};

} // namespace sdk
