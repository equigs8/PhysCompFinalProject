#pragma once

#include "Game.h"

namespace sdk {

constexpr int MAX_REGISTERED_GAMES = 12;

/**
 * @brief Central registry of games shown in the handheld's main menu.
 */
class GameRegistry {
public:
  static bool registerGame(Game* game) {
    if (!game || count_ >= MAX_REGISTERED_GAMES) {
      return false;
    }
    games_[count_++] = game;
    return true;
  }

  static int count() { return count_; }

  static Game* get(int index) {
    if (index < 0 || index >= count_) return nullptr;
    return games_[index];
  }

private:
  static inline Game* games_[MAX_REGISTERED_GAMES] = {};
  static inline int count_ = 0;
};

} // namespace sdk
