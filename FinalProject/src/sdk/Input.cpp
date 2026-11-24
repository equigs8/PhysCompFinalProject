#include "Input.h"
#include "BoardConfig.h"

namespace sdk {

void Input::begin() {
  pins_[BTN_UP]     = PIN_UP;
  pins_[BTN_DOWN]   = PIN_DOWN;
  pins_[BTN_LEFT]   = PIN_LEFT;
  pins_[BTN_RIGHT]  = PIN_RIGHT;
  pins_[BTN_A]      = PIN_BUTTONA;
  pins_[BTN_B]      = PIN_BUTTONB;
  pins_[BTN_SELECT] = PIN_SELECT;
  pins_[BTN_HOME]   = PIN_HOME;

  unsigned long now = millis();
  lastUpdateMs_ = now;
  for (uint8_t i = 0; i < BTN_COUNT; ++i) {
    pinMode(pins_[i], INPUT_PULLUP);
    bool down = (digitalRead(pins_[i]) == LOW);
    rawState_[i] = down;
    stableState_[i] = down;
    prevStableState_[i] = down;
    lastDebounceMs_[i] = now;
    pressStartMs_[i] = now;
    lastRepeatMs_[i] = now;
  }
}

void Input::update(unsigned long nowMs) {
  lastUpdateMs_ = nowMs;
  for (uint8_t i = 0; i < BTN_COUNT; ++i) {
    prevStableState_[i] = stableState_[i];
    bool reading = (digitalRead(pins_[i]) == LOW);

    if (reading != rawState_[i]) {
      rawState_[i] = reading;
      lastDebounceMs_[i] = nowMs;
    }

    if ((nowMs - lastDebounceMs_[i]) >= DEBOUNCE_MS) {
      if (reading != stableState_[i]) {
        stableState_[i] = reading;
        if (stableState_[i]) {
          pressStartMs_[i] = nowMs;
          lastRepeatMs_[i] = nowMs;
        }
      }
    }
  }
}

void Input::clearEdges() {
  for (uint8_t i = 0; i < BTN_COUNT; ++i) {
    prevStableState_[i] = stableState_[i];
  }
}

bool Input::pressed(Button b) const {
  if (b >= BTN_COUNT) return false;
  return stableState_[b] && !prevStableState_[b];
}

bool Input::held(Button b) const {
  if (b >= BTN_COUNT) return false;
  return stableState_[b];
}

bool Input::released(Button b) const {
  if (b >= BTN_COUNT) return false;
  return !stableState_[b] && prevStableState_[b];
}

bool Input::repeated(Button b, uint16_t initialDelayMs, uint16_t repeatMs) {
  if (b >= BTN_COUNT) return false;
  if (pressed(b)) {
    lastRepeatMs_[b] = lastUpdateMs_;
    return true;
  }
  if (!stableState_[b]) return false;

  if ((lastUpdateMs_ - pressStartMs_[b]) >= initialDelayMs &&
      (lastUpdateMs_ - lastRepeatMs_[b]) >= repeatMs) {
    lastRepeatMs_[b] = lastUpdateMs_;
    return true;
  }
  return false;
}

int Input::axisX() const {
  int x = 0;
  if (held(BTN_LEFT))  x -= 1;
  if (held(BTN_RIGHT)) x += 1;
  return x;
}

int Input::axisY() const {
  int y = 0;
  if (held(BTN_UP))   y -= 1;
  if (held(BTN_DOWN)) y += 1;
  return y;
}

} // namespace sdk
