#include "Console.h"

namespace sdk {

Console::Console(Adafruit_ILI9341& tft) : display(tft) {}

void Console::begin() {
  display.begin();
  input.begin();
  audio.begin();
  lastUpdateMs_ = millis();
  lastRenderMs_ = lastUpdateMs_;
}

void Console::launchGame(Game* game) {
  if (!game) return;
  if (activeGame_) {
    activeGame_->onExit(*this);
  }

  activeGame_ = game;
  exitRequested_ = false;
  display.setBuffered(game->usesFramebuffer());

  unsigned long now = millis();
  input.update(now);
  input.clearEdges(); // Prevent menu-select press from triggering inside frame 1

  lastUpdateMs_ = now;
  lastRenderMs_ = now;

  activeGame_->onEnter(*this);

  if (activeGame_ && activeGame_->usesFramebuffer()) {
    activeGame_->onRender(*this);
    display.present();
    lastRenderMs_ = millis();
  }
}

void Console::exitGame() {
  if (activeGame_) {
    activeGame_->onExit(*this);
    activeGame_ = nullptr;
  }
  exitRequested_ = false;
  audio.stop();
  display.setBuffered(false);
}

void Console::tick() {
  unsigned long now = millis();
  input.update(now);
  audio.update(now);

  if (!activeGame_) {
    lastUpdateMs_ = now;
    return;
  }

  float dt = static_cast<float>(now - lastUpdateMs_) / 1000.0f;
  if (dt > 0.1f) dt = 0.1f; // Clamp delta time after pauses
  lastUpdateMs_ = now;

  activeGame_->onUpdate(*this, dt);
  if (!activeGame_) return; // Game may have exited itself

  if (activeGame_->usesFramebuffer()) {
    uint16_t interval = activeGame_->renderIntervalMs();
    if (interval == 0 || (now - lastRenderMs_) >= interval) {
      activeGame_->onRender(*this);
      display.present();
      lastRenderMs_ = now;
    }
  } else {
    activeGame_->onRender(*this);
  }
}

} // namespace sdk
