#pragma once

#include "../../sdk/Game.h"
#include "../../sdk/Console.h"
#include "assets/icon.h"

// ==============================================================================
// STARTER TEMPLATE GAME
// ==============================================================================
// Copy this entire folder (src/games/template/ -> src/games/my_game/),
// rename TemplateGame -> MyGame, and register it in src/games/GameList.h!
//
// Tip: Generate your own 16x16 menu icon into assets/icon.h using:
//   - tools/sprite-studio.html (in any browser), OR
//   - python convert_image.py my_icon.png --icon --name MY_ICON -o src/games/my_game/assets/icon.h

class TemplateGame : public sdk::Game {
public:
  const char* title() const override { return "My Game"; }

  sdk::Sprite icon() const override { return TEMPLATE_ICON; }

  void onEnter(sdk::Console& console) override {
    (void)console;
    playerX_ = 150.0f;
    playerY_ = 110.0f;
    score_ = 0;
    colorIndex_ = 0;
  }

  void onUpdate(sdk::Console& console, float dt) override {
    // Smooth D-pad movement using delta time (pixels per second)
    const float speed = 120.0f;
    playerX_ += console.input.axisX() * speed * dt;
    playerY_ += console.input.axisY() * speed * dt;

    // Keep player inside the screen
    playerX_ = constrain(playerX_, 8.0f, 320.0f - 28.0f);
    playerY_ = constrain(playerY_, 30.0f, 240.0f - 28.0f);

    // Edge-triggered A button press
    if (console.input.pressed(sdk::BTN_A)) {
      ++score_;
      colorIndex_ = (colorIndex_ + 1) % 4;
      console.audio.playSelect();
    }
  }

  void onRender(sdk::Console& console) override {
    auto& disp = console.display;
    auto& g = disp.gfx();

    disp.clear(sdk::Colors::PanelBg);

    // Header
    char buf[32];
    snprintf(buf, sizeof(buf), "MY GAME  |  SCORE: %d", score_);
    disp.drawCenteredText(buf, 8, sdk::Colors::White, 1);
    disp.drawCenteredText("D-PAD: Move   A: Color+Score   HOME: Exit", 224, sdk::Colors::LightGray, 1);

    // Draw player box
    static const uint16_t palette[4] = {
      sdk::Colors::Cyan, sdk::Colors::Yellow, sdk::Colors::Green, sdk::Colors::Orange
    };
    g.fillRoundRect(static_cast<int>(playerX_), static_cast<int>(playerY_), 20, 20, 4, palette[colorIndex_]);
  }

private:
  float playerX_ = 150.0f;
  float playerY_ = 110.0f;
  int score_ = 0;
  int colorIndex_ = 0;
};
