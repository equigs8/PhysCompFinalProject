#include "Audio.h"
#include "BoardConfig.h"

namespace sdk {

void Audio::begin() {
#if defined(ENABLE_AUDIO)
  ledcAttach(PIN_PIEZO, 2000, 8);
  ledcWrite(PIN_PIEZO, 0);
#endif
}

void Audio::tone(uint16_t freqHz, uint16_t durationMs) {
#if defined(ENABLE_AUDIO)
  if (freqHz == 0 || durationMs == 0) {
    stop();
    return;
  }
  ledcWriteTone(PIN_PIEZO, freqHz);
  ledcWrite(PIN_PIEZO, 96); // Safe moderate volume duty cycle
  playing_ = true;
  stopAtMs_ = millis() + durationMs;
#else
  (void)freqHz;
  (void)durationMs;
#endif
}

void Audio::update(unsigned long nowMs) {
#if defined(ENABLE_AUDIO)
  if (playing_ && static_cast<long>(nowMs - stopAtMs_) >= 0) {
    stop();
  }
#else
  (void)nowMs;
#endif
}

void Audio::stop() {
#if defined(ENABLE_AUDIO)
  ledcWrite(PIN_PIEZO, 0);
  playing_ = false;
#endif
}

} // namespace sdk
