#pragma once

#include "../../sdk/Game.h"
#include "../../sdk/Console.h"
#include "assets/icon.h"

/**
 * @brief Playable Snake example built entirely with the Handheld SDK.
 *        Demonstrates flicker-free framebuffer rendering, edge-triggered input,
 *        delta-time movement, HUD drawing, and audio SFX.
 */
class SnakeGame : public sdk::Game {
public:
  const char* title() const override { return "Snake"; }

  sdk::Sprite icon() const override { return SNAKE_ICON; }

  void onEnter(sdk::Console& console) override {
    (void)console;
    randomSeed(micros());
    resetRound();
  }

  void onUpdate(sdk::Console& console, float dt) override {
    if (gameOver_) {
      if (console.input.pressed(sdk::BTN_A)) {
        console.audio.playSelect();
        resetRound();
      }
      return;
    }

    // Queue next direction (prevent 180-degree reversal into own neck)
    if (console.input.pressed(sdk::BTN_UP) && dirY_ != 1) {
      nextDirX_ = 0; nextDirY_ = -1;
    } else if (console.input.pressed(sdk::BTN_DOWN) && dirY_ != -1) {
      nextDirX_ = 0; nextDirY_ = 1;
    } else if (console.input.pressed(sdk::BTN_LEFT) && dirX_ != 1) {
      nextDirX_ = -1; nextDirY_ = 0;
    } else if (console.input.pressed(sdk::BTN_RIGHT) && dirX_ != -1) {
      nextDirX_ = 1; nextDirY_ = 0;
    }

    stepTimer_ += dt;
    float stepInterval = max(0.075f, 0.145f - score_ * 0.003f);
    if (stepTimer_ < stepInterval) {
      return;
    }
    stepTimer_ -= stepInterval;

    dirX_ = nextDirX_;
    dirY_ = nextDirY_;

    int newX = bodyX_[0] + dirX_;
    int newY = bodyY_[0] + dirY_;

    // Wall collision
    if (newX < 0 || newX >= GRID_W || newY < 0 || newY >= GRID_H) {
      triggerGameOver(console);
      return;
    }

    bool eating = (newX == foodX_ && newY == foodY_);
    int checkLen = eating ? length_ : (length_ - 1);

    // Self collision
    for (int i = 0; i < checkLen; ++i) {
      if (bodyX_[i] == newX && bodyY_[i] == newY) {
        triggerGameOver(console);
        return;
      }
    }

    if (eating && length_ < MAX_LEN) {
      ++length_;
    }

    // Shift body segments
    for (int i = length_ - 1; i > 0; --i) {
      bodyX_[i] = bodyX_[i - 1];
      bodyY_[i] = bodyY_[i - 1];
    }
    bodyX_[0] = newX;
    bodyY_[0] = newY;

    if (eating) {
      ++score_;
      if (score_ > bestScore_) bestScore_ = score_;
      console.audio.playPickup();
      spawnFood();
    }
  }

  void onRender(sdk::Console& console) override {
    auto& disp = console.display;
    auto& g = disp.gfx();

    // Background & top HUD bar
    disp.clear(sdk::Colors::rgb(12, 16, 24));
    disp.drawPanel(0, 0, 320, 24, sdk::Colors::PanelBg, sdk::Colors::PanelLine);

    char hudBuf[32];
    snprintf(hudBuf, sizeof(hudBuf), "SCORE: %d", score_);
    disp.drawText(hudBuf, 10, 8, sdk::Colors::White, 1);

    snprintf(hudBuf, sizeof(hudBuf), "BEST: %d", bestScore_);
    disp.drawCenteredText(hudBuf, 8, sdk::Colors::Yellow, 1);
    disp.drawText("HOME: Menu", 252, 8, sdk::Colors::LightGray, 1);

    // Arena box
    const int arenaW = GRID_W * CELL;
    const int arenaH = GRID_H * CELL;
    disp.drawPanel(ARENA_X - 2, ARENA_Y - 2, arenaW + 4, arenaH + 4,
                   sdk::Colors::rgb(20, 26, 36), sdk::Colors::rgb(64, 80, 108));

    // Subtle checkerboard cells
    const uint16_t tileAlt = sdk::Colors::rgb(24, 31, 44);
    for (int gy = 0; gy < GRID_H; ++gy) {
      for (int gx = 0; gx < GRID_W; ++gx) {
        if (((gx + gy) & 1) == 0) {
          g.fillRect(ARENA_X + gx * CELL, ARENA_Y + gy * CELL, CELL, CELL, tileAlt);
        }
      }
    }

    // Food apple
    int fx = ARENA_X + foodX_ * CELL;
    int fy = ARENA_Y + foodY_ * CELL;
    g.fillRoundRect(fx + 2, fy + 3, CELL - 4, CELL - 4, 3, sdk::Colors::Red);
    g.fillRect(fx + CELL / 2 - 1, fy + 1, 3, 2, sdk::Colors::Green);
    g.drawPixel(fx + 4, fy + 5, sdk::Colors::White);

    // Snake segments
    for (int i = length_ - 1; i >= 0; --i) {
      int sx = ARENA_X + bodyX_[i] * CELL;
      int sy = ARENA_Y + bodyY_[i] * CELL;
      uint16_t segCol = (i == 0)
        ? sdk::Colors::rgb(90, 245, 110)
        : ((i & 1) ? sdk::Colors::rgb(46, 196, 82) : sdk::Colors::rgb(36, 168, 68));
      g.fillRoundRect(sx + 1, sy + 1, CELL - 2, CELL - 2, 3, segCol);

      // Eyes on the snake head
      if (i == 0) {
        int e1x = sx + 4, e1y = sy + 4;
        int e2x = sx + CELL - 6, e2y = sy + 4;
        if (dirX_ == 1)       { e1x = sx + CELL - 5; e1y = sy + 3; e2x = sx + CELL - 5; e2y = sy + CELL - 5; }
        else if (dirX_ == -1) { e1x = sx + 3;        e1y = sy + 3; e2x = sx + 3;        e2y = sy + CELL - 5; }
        else if (dirY_ == 1)  { e1x = sx + 3;        e1y = sy + CELL - 5; e2x = sx + CELL - 5; e2y = sy + CELL - 5; }
        g.fillRect(e1x, e1y, 2, 2, sdk::Colors::Black);
        g.fillRect(e2x, e2y, 2, 2, sdk::Colors::Black);
      }
    }

    // Game Over modal card
    if (gameOver_) {
      disp.drawPanel(60, 72, 200, 96, sdk::Colors::PanelBg, sdk::Colors::Red);
      disp.drawCenteredText("GAME OVER", 88, sdk::Colors::Red, 2);

      snprintf(hudBuf, sizeof(hudBuf), "Score: %d  Best: %d", score_, bestScore_);
      disp.drawCenteredText(hudBuf, 118, sdk::Colors::White, 1);
      disp.drawCenteredText("Press A to Play Again", 142, sdk::Colors::Yellow, 1);
    }
  }

private:
  static constexpr int CELL = 14;
  static constexpr int GRID_W = 20;   // 280 px wide
  static constexpr int GRID_H = 14;   // 196 px tall
  static constexpr int ARENA_X = (320 - GRID_W * CELL) / 2;
  static constexpr int ARENA_Y = 34;
  static constexpr int MAX_LEN = GRID_W * GRID_H;

  int bodyX_[MAX_LEN] = {};
  int bodyY_[MAX_LEN] = {};
  int length_ = 4;
  int dirX_ = 1, dirY_ = 0;
  int nextDirX_ = 1, nextDirY_ = 0;
  int foodX_ = 14, foodY_ = 7;
  int score_ = 0;
  int bestScore_ = 0;
  float stepTimer_ = 0.0f;
  bool gameOver_ = false;

  void resetRound() {
    length_ = 4;
    dirX_ = 1; dirY_ = 0;
    nextDirX_ = 1; nextDirY_ = 0;
    score_ = 0;
    stepTimer_ = 0.0f;
    gameOver_ = false;

    int startX = 7;
    int startY = GRID_H / 2;
    for (int i = 0; i < length_; ++i) {
      bodyX_[i] = startX - i;
      bodyY_[i] = startY;
    }
    spawnFood();
  }

  void triggerGameOver(sdk::Console& console) {
    gameOver_ = true;
    console.audio.playLose();
  }

  void spawnFood() {
    for (int attempt = 0; attempt < 200; ++attempt) {
      int rx = random(0, GRID_W);
      int ry = random(0, GRID_H);
      bool occupied = false;
      for (int i = 0; i < length_; ++i) {
        if (bodyX_[i] == rx && bodyY_[i] == ry) {
          occupied = true;
          break;
        }
      }
      if (!occupied) {
        foodX_ = rx;
        foodY_ = ry;
        return;
      }
    }
  }
};
