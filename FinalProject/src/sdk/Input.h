#pragma once

#include <Arduino.h>

namespace sdk {

enum Button : uint8_t {
  BTN_UP = 0,
  BTN_DOWN,
  BTN_LEFT,
  BTN_RIGHT,
  BTN_A,
  BTN_B,
  BTN_SELECT,
  BTN_HOME,
  BTN_COUNT
};

/**
 * @brief Debounced, edge-triggered input manager for all handheld buttons.
 */
class Input {
public:
  void begin();
  void update(unsigned long nowMs);
  void clearEdges();

  // True only on the single frame the button transitions from up -> down
  bool pressed(Button b) const;
  // True while the button is currently held down
  bool held(Button b) const;
  // True only on the single frame the button transitions from down -> up
  bool released(Button b) const;
  // True on initial press, then periodically while held (great for menus/grids)
  bool repeated(Button b, uint16_t initialDelayMs = 240, uint16_t repeatMs = 110);

  // Returns -1 (LEFT), +1 (RIGHT), or 0
  int axisX() const;
  // Returns -1 (UP), +1 (DOWN), or 0
  int axisY() const;

private:
  static constexpr uint16_t DEBOUNCE_MS = 20;

  uint8_t pins_[BTN_COUNT] = {};
  bool rawState_[BTN_COUNT] = {};
  bool stableState_[BTN_COUNT] = {};
  bool prevStableState_[BTN_COUNT] = {};
  unsigned long lastDebounceMs_[BTN_COUNT] = {};
  unsigned long pressStartMs_[BTN_COUNT] = {};
  unsigned long lastRepeatMs_[BTN_COUNT] = {};
  unsigned long lastUpdateMs_ = 0;
};

} // namespace sdk
