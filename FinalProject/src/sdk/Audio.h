#pragma once

#include <Arduino.h>

namespace sdk {

/**
 * @brief Non-blocking piezo audio helper.
 *        Define ENABLE_AUDIO in build_flags to drive PIN_PIEZO.
 */
class Audio {
public:
  void begin();
  void update(unsigned long nowMs);
  void tone(uint16_t freqHz, uint16_t durationMs);
  void stop();

  // Convenient retro SFX presets
  void playSelect() { tone(880, 45); }
  void playMove()   { tone(520, 25); }
  void playPickup() { tone(1175, 65); }
  void playHit()    { tone(180, 120); }
  void playWin()    { tone(1047, 150); }
  void playLose()   { tone(220, 220); }

private:
  bool playing_ = false;
  unsigned long stopAtMs_ = 0;
};

} // namespace sdk
