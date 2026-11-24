#pragma once

#include "../../sdk/Game.h"
#include "../../sdk/Console.h"

class ChessGame : public sdk::Game {
public:
  const char* title() const override { return "Chess"; }
  sdk::Sprite icon() const override;
  bool usesFramebuffer() const override { return false; }

  void onEnter(sdk::Console& console) override;
  void onUpdate(sdk::Console& console, float dt) override;
  void onRender(sdk::Console& console) override { (void)console; }
};
