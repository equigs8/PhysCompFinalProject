#pragma once

#include <Arduino.h>
#include <Adafruit_ILI9341.h>
#include "Display.h"
#include "Input.h"
#include "Audio.h"
#include "Game.h"

namespace sdk {

/**
 * @brief Handheld runtime passed to every game (provides Display, Input, Audio).
 */
class Console {
public:
  explicit Console(Adafruit_ILI9341& tft);

  void begin();

  void launchGame(Game* game);
  void exitGame();
  void requestExit() { exitRequested_ = true; }
  bool exitRequested() const { return exitRequested_; }
  bool hasActiveGame() const { return activeGame_ != nullptr; }
  Game* activeGame() const { return activeGame_; }

  // Runs one iteration of input, audio, game update, and paced rendering
  void tick();

  Display display;
  Input input;
  Audio audio;

private:
  Game* activeGame_ = nullptr;
  bool exitRequested_ = false;
  unsigned long lastUpdateMs_ = 0;
  unsigned long lastRenderMs_ = 0;
};

} // namespace sdk
