#pragma once

#include "../../sdk/Game.h"
#include "../../sdk/Console.h"
#include "assets/icon.h"
#include <math.h>

/**
 * @brief Star Blaster — Horizontal arcade space shooter built with the Handheld SDK.
 *
 * Controls:
 *   - D-Pad: Fly 8-way
 *   - A (hold/tap): Fire plasma laser (3-way when Spread power-up is active)
 *   - B: Trigger Nova Shockwave when charge bar is full
 *   - HOME: Return to Main Menu
 */
class StarBlasterGame : public sdk::Game {
public:
  const char* title() const override { return "Star Blaster"; }

  sdk::Sprite icon() const override { return STAR_BLASTER_ICON; }

  void onEnter(sdk::Console& console) override {
    (void)console;
    randomSeed(micros());
    initStars();
    resetGame();
  }

  void onUpdate(sdk::Console& console, float dt) override {
    time_ += dt;
    updateStars(dt);
    updateParticles(dt);

    if (gameOver_) {
      if (console.input.pressed(sdk::BTN_A)) {
        console.audio.playSelect();
        resetGame();
      }
      return;
    }

    // Timers
    if (fireCooldown_ > 0.0f) fireCooldown_ -= dt;
    if (invulnTimer_ > 0.0f) invulnTimer_ -= dt;
    if (spreadTimer_ > 0.0f) spreadTimer_ -= dt;
    if (waveBannerTimer_ > 0.0f) waveBannerTimer_ -= dt;
    if (hitFlashTimer_ > 0.0f) hitFlashTimer_ -= dt;

    // Nova shockwave animation
    if (novaRadius_ > 0.0f) {
      novaRadius_ += 340.0f * dt;
      if (novaRadius_ > 340.0f) {
        novaRadius_ = 0.0f;
      }
    }

    // Player movement
    const float shipSpeed = 148.0f;
    playerX_ += console.input.axisX() * shipSpeed * dt;
    playerY_ += console.input.axisY() * shipSpeed * dt;
    playerX_ = constrain(playerX_, 8.0f, 220.0f);
    playerY_ = constrain(playerY_, (float)(HUD_H + 4), 240.0f - 20.0f);

    // Firing (hold or tap A)
    if ((console.input.held(sdk::BTN_A) || console.input.pressed(sdk::BTN_A)) && fireCooldown_ <= 0.0f) {
      firePlayerWeapon(console);
    }

    // Nova Bomb on B (when charge is 100%)
    if (console.input.pressed(sdk::BTN_B) && novaCharge_ >= 100.0f) {
      triggerNova(console);
    }

    // Wave progression
    int targetWave = 1 + (score_ / 1200);
    if (targetWave > wave_) {
      wave_ = targetWave;
      waveBannerTimer_ = 1.6f;
      console.audio.playWin();
    }

    // Spawn enemies
    spawnTimer_ -= dt;
    if (spawnTimer_ <= 0.0f) {
      spawnEnemy();
      float baseRate = max(0.42f, 1.25f - (wave_ - 1) * 0.11f);
      spawnTimer_ = baseRate * (0.75f + (random(0, 50) / 100.0f));
    }

    // Passive nova recharge
    novaCharge_ = min(100.0f, novaCharge_ + 4.5f * dt);

    updatePlayerBullets(dt);
    updateEnemyBullets(dt, console);
    updateEnemies(dt, console);
    updatePowerups(dt, console);
  }

  void onRender(sdk::Console& console) override {
    auto& disp = console.display;
    auto& g = disp.gfx();

    // Deep space background (subtle red tint during hit flash)
    uint16_t bgCol = (hitFlashTimer_ > 0.0f)
      ? sdk::Colors::rgb(42, 10, 14)
      : sdk::Colors::rgb(6, 10, 20);
    disp.clear(bgCol);

    // 3-layer parallax starfield
    for (int i = 0; i < MAX_STARS; ++i) {
      uint16_t starCol = (stars_[i].layer == 0)
        ? sdk::Colors::rgb(65, 78, 105)
        : (stars_[i].layer == 1 ? sdk::Colors::rgb(130, 155, 200) : sdk::Colors::White);
      g.drawPixel((int)stars_[i].x, (int)stars_[i].y, starCol);
      if (stars_[i].layer == 2) {
        g.drawPixel((int)stars_[i].x - 1, (int)stars_[i].y, sdk::Colors::rgb(120, 160, 220));
      }
    }

    // Nova expanding shockwave ring
    if (novaRadius_ > 0.0f) {
      int r = (int)novaRadius_;
      g.drawCircle((int)novaX_, (int)novaY_, r, sdk::Colors::Cyan);
      if (r > 3) g.drawCircle((int)novaX_, (int)novaY_, r - 2, sdk::Colors::White);
    }

    // Power-ups
    for (int i = 0; i < MAX_POWERUPS; ++i) {
      if (!powerups_[i].active) continue;
      int px = (int)powerups_[i].x;
      int py = (int)powerups_[i].y;
      bool pulse = ((int)(time_ * 8.0f) & 1) == 0;
      uint16_t col = (powerups_[i].type == 0) ? sdk::Colors::Cyan : sdk::Colors::Green;
      g.fillCircle(px, py, 6, col);
      g.drawCircle(px, py, pulse ? 8 : 7, sdk::Colors::White);
      disp.drawText(powerups_[i].type == 0 ? "S" : "+", px - 2, py - 3, sdk::Colors::Black, 1);
    }

    // Enemies
    for (int i = 0; i < MAX_ENEMIES; ++i) {
      if (!enemies_[i].active) continue;
      drawEnemy(g, enemies_[i]);
    }

    // Player lasers
    for (int i = 0; i < MAX_PBULLETS; ++i) {
      if (!pBullets_[i].active) continue;
      int bx = (int)pBullets_[i].x;
      int by = (int)pBullets_[i].y;
      uint16_t col = (spreadTimer_ > 0.0f) ? sdk::Colors::Yellow : sdk::Colors::Cyan;
      g.fillRect(bx, by - 1, 8, 3, col);
      g.fillRect(bx + 2, by, 5, 1, sdk::Colors::White);
    }

    // Enemy plasma orbs
    for (int i = 0; i < MAX_EBULLETS; ++i) {
      if (!eBullets_[i].active) continue;
      int bx = (int)eBullets_[i].x;
      int by = (int)eBullets_[i].y;
      g.fillCircle(bx, by, 3, sdk::Colors::Red);
      g.drawPixel(bx, by, sdk::Colors::Yellow);
    }

    // Player ship (blinks while invulnerable)
    if (!gameOver_) {
      bool visible = (invulnTimer_ <= 0.0f) || (((int)(time_ * 16.0f) & 1) == 0);
      if (visible) {
        int sx = (int)playerX_;
        int sy = (int)playerY_;
        // Animated thruster flicker behind ship
        int flameLen = 3 + ((int)(time_ * 24.0f) % 4);
        g.fillTriangle(sx, sy + 5, sx, sy + 10, sx - flameLen, sy + 7, sdk::Colors::Orange);
        disp.drawSprite(icon(), sx, sy, 1);

        // Shield halo when invulnerable
        if (invulnTimer_ > 0.0f) {
          g.drawCircle(sx + 8, sy + 8, 11, sdk::Colors::Cyan);
        }
      }
    }

    // Explosion particles
    for (int i = 0; i < MAX_PARTICLES; ++i) {
      if (!particles_[i].active) continue;
      int size = (particles_[i].life > 0.18f) ? 2 : 1;
      g.fillRect((int)particles_[i].x, (int)particles_[i].y, size, size, particles_[i].color);
    }

    // Wave banner
    if (!gameOver_ && waveBannerTimer_ > 0.0f) {
      char waveBuf[20];
      snprintf(waveBuf, sizeof(waveBuf), "WAVE %d", wave_);
      disp.drawPanel(104, 96, 112, 34, sdk::Colors::PanelBg, sdk::Colors::Cyan);
      disp.drawCenteredText(waveBuf, 106, sdk::Colors::Cyan, 2);
    }

    // Top HUD Bar
    disp.drawPanel(0, 0, 320, HUD_H, sdk::Colors::PanelBg, sdk::Colors::PanelLine);

    char hudBuf[32];
    snprintf(hudBuf, sizeof(hudBuf), "SCORE %d", score_);
    disp.drawText(hudBuf, 8, 7, sdk::Colors::White, 1);

    snprintf(hudBuf, sizeof(hudBuf), "W%d", wave_);
    disp.drawText(hudBuf, 98, 7, sdk::Colors::Yellow, 1);

    // Nova charge bar (or Spread countdown bar)
    if (spreadTimer_ > 0.0f) {
      disp.drawText("SPREAD", 128, 7, sdk::Colors::Yellow, 1);
      int fillW = (int)((spreadTimer_ / 9.0f) * 52.0f);
      g.drawRect(170, 7, 54, 8, sdk::Colors::Yellow);
      g.fillRect(171, 8, constrain(fillW, 0, 52), 6, sdk::Colors::Orange);
    } else {
      bool ready = (novaCharge_ >= 100.0f);
      disp.drawText(ready ? "NOVA[B]" : "NOVA", 128, 7, ready ? sdk::Colors::Cyan : sdk::Colors::LightGray, 1);
      int fillW = (int)((novaCharge_ / 100.0f) * 50.0f);
      g.drawRect(174, 7, 52, 8, ready ? sdk::Colors::White : sdk::Colors::PanelLine);
      g.fillRect(175, 8, constrain(fillW, 0, 50), 6, ready ? sdk::Colors::Cyan : sdk::Colors::Blue);
    }

    // Lives pips on right side of HUD
    for (int i = 0; i < 3; ++i) {
      int lx = 264 + i * 16;
      uint16_t pipCol = (i < lives_) ? sdk::Colors::Red : sdk::Colors::DarkGray;
      g.fillTriangle(lx, 7, lx, 15, lx + 10, 11, pipCol);
    }

    // Game Over modal
    if (gameOver_) {
      disp.drawPanel(52, 62, 216, 116, sdk::Colors::PanelBg, sdk::Colors::Red);
      disp.drawCenteredText("MISSION FAILED", 78, sdk::Colors::Red, 2);

      snprintf(hudBuf, sizeof(hudBuf), "SCORE: %d   BEST: %d", score_, bestScore_);
      disp.drawCenteredText(hudBuf, 108, sdk::Colors::White, 1);

      snprintf(hudBuf, sizeof(hudBuf), "REACHED WAVE %d", wave_);
      disp.drawCenteredText(hudBuf, 126, sdk::Colors::Cyan, 1);

      disp.drawCenteredText("Press A to Fly Again", 152, sdk::Colors::Yellow, 1);
    }
  }

private:
  static constexpr int HUD_H = 22;
  static constexpr int MAX_STARS = 28;
  static constexpr int MAX_PBULLETS = 20;
  static constexpr int MAX_EBULLETS = 16;
  static constexpr int MAX_ENEMIES = 12;
  static constexpr int MAX_PARTICLES = 36;
  static constexpr int MAX_POWERUPS = 3;

  struct Star { float x, y, speed; uint8_t layer; };
  struct Bullet { float x, y, vx, vy; bool active; };
  struct Enemy {
    float x, y, baseY, vx, phase, shootTimer, flashTimer;
    int hp;
    uint8_t type; // 0 = Scout, 1 = Interceptor, 2 = Heavy Cruiser
    bool active;
  };
  struct Particle { float x, y, vx, vy, life; uint16_t color; bool active; };
  struct Powerup { float x, y; uint8_t type; bool active; }; // 0 = Spread, 1 = Life

  Star stars_[MAX_STARS] = {};
  Bullet pBullets_[MAX_PBULLETS] = {};
  Bullet eBullets_[MAX_EBULLETS] = {};
  Enemy enemies_[MAX_ENEMIES] = {};
  Particle particles_[MAX_PARTICLES] = {};
  Powerup powerups_[MAX_POWERUPS] = {};

  float playerX_ = 28.0f;
  float playerY_ = 120.0f;
  float time_ = 0.0f;
  float fireCooldown_ = 0.0f;
  float spawnTimer_ = 0.6f;
  float invulnTimer_ = 0.0f;
  float spreadTimer_ = 0.0f;
  float waveBannerTimer_ = 1.4f;
  float hitFlashTimer_ = 0.0f;
  float novaCharge_ = 50.0f;
  float novaRadius_ = 0.0f;
  float novaX_ = 0.0f;
  float novaY_ = 0.0f;

  int lives_ = 3;
  int score_ = 0;
  int bestScore_ = 0;
  int wave_ = 1;
  bool gameOver_ = false;

  void initStars() {
    for (int i = 0; i < MAX_STARS; ++i) {
      stars_[i].x = (float)random(0, 320);
      stars_[i].y = (float)random(HUD_H + 2, 238);
      stars_[i].layer = (uint8_t)(i % 3);
      stars_[i].speed = (stars_[i].layer == 0) ? 28.0f : (stars_[i].layer == 1 ? 60.0f : 110.0f);
    }
  }

  void resetGame() {
    playerX_ = 28.0f;
    playerY_ = 120.0f;
    fireCooldown_ = 0.0f;
    spawnTimer_ = 0.6f;
    invulnTimer_ = 1.0f;
    spreadTimer_ = 0.0f;
    waveBannerTimer_ = 1.4f;
    hitFlashTimer_ = 0.0f;
    novaCharge_ = 50.0f;
    novaRadius_ = 0.0f;
    lives_ = 3;
    score_ = 0;
    wave_ = 1;
    gameOver_ = false;

    for (int i = 0; i < MAX_PBULLETS; ++i) pBullets_[i].active = false;
    for (int i = 0; i < MAX_EBULLETS; ++i) eBullets_[i].active = false;
    for (int i = 0; i < MAX_ENEMIES; ++i)  enemies_[i].active = false;
    for (int i = 0; i < MAX_POWERUPS; ++i) powerups_[i].active = false;
    for (int i = 0; i < MAX_PARTICLES; ++i) particles_[i].active = false;
  }

  void updateStars(float dt) {
    for (int i = 0; i < MAX_STARS; ++i) {
      stars_[i].x -= stars_[i].speed * dt;
      if (stars_[i].x < 0.0f) {
        stars_[i].x = 319.0f;
        stars_[i].y = (float)random(HUD_H + 2, 238);
      }
    }
  }

  void spawnPlayerBullet(float x, float y, float vx, float vy) {
    for (int i = 0; i < MAX_PBULLETS; ++i) {
      if (!pBullets_[i].active) {
        pBullets_[i] = { x, y, vx, vy, true };
        return;
      }
    }
  }

  void firePlayerWeapon(sdk::Console& console) {
    fireCooldown_ = 0.16f;
    float muzzleX = playerX_ + 15.0f;
    float muzzleY = playerY_ + 8.0f;
    if (spreadTimer_ > 0.0f) {
      spawnPlayerBullet(muzzleX, muzzleY, 270.0f, 0.0f);
      spawnPlayerBullet(muzzleX, muzzleY - 2.0f, 255.0f, -68.0f);
      spawnPlayerBullet(muzzleX, muzzleY + 2.0f, 255.0f, 68.0f);
    } else {
      spawnPlayerBullet(muzzleX, muzzleY, 285.0f, 0.0f);
    }
    console.audio.playMove();
  }

  void triggerNova(sdk::Console& console) {
    novaCharge_ = 0.0f;
    novaX_ = playerX_ + 8.0f;
    novaY_ = playerY_ + 8.0f;
    novaRadius_ = 8.0f;
    console.audio.playWin();

    // Clear all enemy bullets
    for (int i = 0; i < MAX_EBULLETS; ++i) {
      if (eBullets_[i].active) {
        spawnExplosion(eBullets_[i].x, eBullets_[i].y, sdk::Colors::Cyan, 3);
        eBullets_[i].active = false;
      }
    }
    // Heavy damage to all visible enemies
    for (int i = 0; i < MAX_ENEMIES; ++i) {
      if (!enemies_[i].active) continue;
      enemies_[i].hp -= 2;
      enemies_[i].flashTimer = 0.12f;
      if (enemies_[i].hp <= 0) {
        destroyEnemy(enemies_[i], console);
      }
    }
  }

  void spawnEnemy() {
    for (int i = 0; i < MAX_ENEMIES; ++i) {
      if (enemies_[i].active) continue;

      int roll = random(0, 100);
      uint8_t type = 0;
      if (wave_ >= 2 && roll > 72) type = 2;      // Heavy Cruiser
      else if (roll > 42)          type = 1;      // Fast Interceptor

      Enemy& e = enemies_[i];
      e.active = true;
      e.type = type;
      e.x = 326.0f;
      e.y = (float)random(HUD_H + 14, 222);
      e.baseY = e.y;
      e.phase = (float)random(0, 628) / 100.0f;
      e.flashTimer = 0.0f;

      float speedBoost = 1.0f + (wave_ - 1) * 0.08f;
      if (type == 0) {
        e.vx = -72.0f * speedBoost;
        e.hp = 1;
        e.shootTimer = 99.0f;
      } else if (type == 1) {
        e.vx = -125.0f * speedBoost;
        e.hp = 1;
        e.shootTimer = 99.0f;
      } else {
        e.vx = -48.0f * speedBoost;
        e.hp = 3;
        e.shootTimer = 0.9f;
      }
      return;
    }
  }

  void spawnEnemyBullet(float x, float y) {
    for (int i = 0; i < MAX_EBULLETS; ++i) {
      if (eBullets_[i].active) continue;
      float dx = (playerX_ + 8.0f) - x;
      float dy = (playerY_ + 8.0f) - y;
      float dist = sqrtf(dx * dx + dy * dy);
      if (dist < 1.0f) dist = 1.0f;
      float speed = 135.0f + wave_ * 6.0f;
      eBullets_[i] = { x, y, (dx / dist) * speed, (dy / dist) * speed, true };
      return;
    }
  }

  void spawnPowerup(float x, float y) {
    for (int i = 0; i < MAX_POWERUPS; ++i) {
      if (powerups_[i].active) continue;
      uint8_t type = (lives_ < 3 && random(0, 100) < 35) ? 1 : 0;
      powerups_[i] = { x, y, type, true };
      return;
    }
  }

  void spawnExplosion(float x, float y, uint16_t baseColor, int count) {
    int spawned = 0;
    for (int i = 0; i < MAX_PARTICLES && spawned < count; ++i) {
      if (particles_[i].active) continue;
      float angle = (float)random(0, 628) / 100.0f;
      float spd = (float)random(25, 115);
      uint16_t c = (spawned % 3 == 0) ? sdk::Colors::White
                 : ((spawned % 3 == 1) ? sdk::Colors::Yellow : baseColor);
      particles_[i] = { x, y, cosf(angle) * spd, sinf(angle) * spd, 0.35f, c, true };
      ++spawned;
    }
  }

  void destroyEnemy(Enemy& e, sdk::Console& console) {
    e.active = false;
    int pts = (e.type == 2) ? 350 : (e.type == 1 ? 150 : 100);
    score_ += pts;
    if (score_ > bestScore_) bestScore_ = score_;
    novaCharge_ = min(100.0f, novaCharge_ + (e.type == 2 ? 20.0f : 9.0f));

    uint16_t col = (e.type == 2) ? sdk::Colors::Orange : sdk::Colors::Magenta;
    spawnExplosion(e.x, e.y, col, e.type == 2 ? 12 : 7);
    console.audio.playHit();

    int dropChance = (e.type == 2) ? 45 : 10;
    if (random(0, 100) < dropChance) {
      spawnPowerup(e.x, e.y);
    }
  }

  void damagePlayer(sdk::Console& console) {
    if (invulnTimer_ > 0.0f || gameOver_) return;
    --lives_;
    invulnTimer_ = 1.6f;
    hitFlashTimer_ = 0.14f;
    spawnExplosion(playerX_ + 8.0f, playerY_ + 8.0f, sdk::Colors::Red, 14);

    if (lives_ <= 0) {
      gameOver_ = true;
      console.audio.playLose();
    } else {
      console.audio.playHit();
    }
  }

  void updatePlayerBullets(float dt) {
    for (int i = 0; i < MAX_PBULLETS; ++i) {
      if (!pBullets_[i].active) continue;
      pBullets_[i].x += pBullets_[i].vx * dt;
      pBullets_[i].y += pBullets_[i].vy * dt;
      if (pBullets_[i].x > 324.0f || pBullets_[i].y < HUD_H || pBullets_[i].y > 244.0f) {
        pBullets_[i].active = false;
      }
    }
  }

  void updateEnemyBullets(float dt, sdk::Console& console) {
    for (int i = 0; i < MAX_EBULLETS; ++i) {
      if (!eBullets_[i].active) continue;
      eBullets_[i].x += eBullets_[i].vx * dt;
      eBullets_[i].y += eBullets_[i].vy * dt;
      if (eBullets_[i].x < -6.0f || eBullets_[i].x > 326.0f ||
          eBullets_[i].y < HUD_H || eBullets_[i].y > 246.0f) {
        eBullets_[i].active = false;
        continue;
      }
      if (fabsf(eBullets_[i].x - (playerX_ + 8.0f)) < 8.0f &&
          fabsf(eBullets_[i].y - (playerY_ + 8.0f)) < 7.0f) {
        eBullets_[i].active = false;
        damagePlayer(console);
      }
    }
  }

  void updateEnemies(float dt, sdk::Console& console) {
    for (int i = 0; i < MAX_ENEMIES; ++i) {
      if (!enemies_[i].active) continue;
      Enemy& e = enemies_[i];
      if (e.flashTimer > 0.0f) e.flashTimer -= dt;

      e.x += e.vx * dt;
      if (e.type == 0) {
        e.phase += 4.2f * dt;
        e.y = constrain(e.baseY + sinf(e.phase) * 26.0f, (float)(HUD_H + 10), 228.0f);
      } else if (e.type == 1) {
        // Gently steers toward player's altitude
        float dy = (playerY_ + 8.0f) - e.y;
        e.y += constrain(dy, -1.0f, 1.0f) * 38.0f * dt;
      } else if (e.type == 2) {
        e.shootTimer -= dt;
        if (e.shootTimer <= 0.0f && e.x < 305.0f && e.x > 60.0f) {
          spawnEnemyBullet(e.x - 8.0f, e.y);
          e.shootTimer = max(0.85f, 1.65f - wave_ * 0.08f);
        }
      }

      if (e.x < -18.0f) {
        e.active = false;
        continue;
      }

      float hitR = (e.type == 2) ? 10.0f : 7.5f;

      // Bullet vs Enemy collisions
      for (int b = 0; b < MAX_PBULLETS && e.active; ++b) {
        if (!pBullets_[b].active) continue;
        if (fabsf((pBullets_[b].x + 4.0f) - e.x) < hitR + 4.0f &&
            fabsf(pBullets_[b].y - e.y) < hitR + 2.0f) {
          pBullets_[b].active = false;
          --e.hp;
          e.flashTimer = 0.08f;
          if (e.hp <= 0) {
            destroyEnemy(e, console);
          } else {
            spawnExplosion(pBullets_[b].x, pBullets_[b].y, sdk::Colors::Yellow, 3);
            console.audio.playMove();
          }
        }
      }

      // Enemy vs Player ship collision
      if (e.active &&
          fabsf(e.x - (playerX_ + 8.0f)) < hitR + 6.0f &&
          fabsf(e.y - (playerY_ + 8.0f)) < hitR + 5.0f) {
        destroyEnemy(e, console);
        damagePlayer(console);
      }
    }
  }

  void updatePowerups(float dt, sdk::Console& console) {
    for (int i = 0; i < MAX_POWERUPS; ++i) {
      if (!powerups_[i].active) continue;
      powerups_[i].x -= 52.0f * dt;
      if (powerups_[i].x < -10.0f) {
        powerups_[i].active = false;
        continue;
      }
      if (fabsf(powerups_[i].x - (playerX_ + 8.0f)) < 13.0f &&
          fabsf(powerups_[i].y - (playerY_ + 8.0f)) < 13.0f) {
        if (powerups_[i].type == 0) {
          spreadTimer_ = 9.0f;
        } else {
          lives_ = min(3, lives_ + 1);
        }
        score_ += 50;
        if (score_ > bestScore_) bestScore_ = score_;
        console.audio.playPickup();
        spawnExplosion(powerups_[i].x, powerups_[i].y, sdk::Colors::Cyan, 6);
        powerups_[i].active = false;
      }
    }
  }

  void updateParticles(float dt) {
    for (int i = 0; i < MAX_PARTICLES; ++i) {
      if (!particles_[i].active) continue;
      particles_[i].x += particles_[i].vx * dt;
      particles_[i].y += particles_[i].vy * dt;
      particles_[i].life -= dt;
      if (particles_[i].life <= 0.0f) {
        particles_[i].active = false;
      }
    }
  }

  void drawEnemy(Adafruit_GFX& g, const Enemy& e) {
    int ex = (int)e.x;
    int ey = (int)e.y;
    bool flash = (e.flashTimer > 0.0f);

    if (e.type == 0) {
      // Scout Drone (magenta diamond)
      uint16_t body = flash ? sdk::Colors::White : sdk::Colors::Magenta;
      g.fillTriangle(ex - 7, ey, ex + 5, ey - 6, ex + 5, ey + 6, body);
      g.fillRect(ex - 2, ey - 2, 4, 4, flash ? sdk::Colors::White : sdk::Colors::Yellow);
    } else if (e.type == 1) {
      // Fast Interceptor (red dart)
      uint16_t body = flash ? sdk::Colors::White : sdk::Colors::Red;
      g.fillTriangle(ex - 8, ey, ex + 7, ey - 5, ex + 3, ey, body);
      g.fillTriangle(ex - 8, ey, ex + 7, ey + 5, ex + 3, ey, body);
      g.drawFastHLine(ex - 4, ey, 6, sdk::Colors::Yellow);
    } else {
      // Heavy Cruiser (armored orange octagon)
      uint16_t body = flash ? sdk::Colors::White : sdk::Colors::Orange;
      g.fillRoundRect(ex - 9, ey - 8, 18, 16, 4, body);
      g.drawRoundRect(ex - 9, ey - 8, 18, 16, 4, sdk::Colors::Yellow);
      g.fillRect(ex - 6, ey - 3, 5, 6, flash ? sdk::Colors::White : sdk::Colors::Red);
      g.fillRect(ex + 2, ey - 4, 4, 8, sdk::Colors::DarkGray);
    }
  }
};
