#pragma once
#include <Arduino.h>
#include "Config.h"

// Drives the button's LED without ever blocking.
// Call only from the main loop — the network task signals through main.cpp.
class LedEngine {
 public:
  void begin();
  void applyPins();  // re-attach after the pin or invert setting changed

  // Continuous state (ready / cooldown / setup ...).
  void setBase(uint8_t mode, uint8_t brightPct, uint16_t periodMs, const char* state);

  // Temporary pattern on top of the base state.
  void play(uint8_t mode, uint8_t brightPct, uint16_t periodMs, uint32_t durationMs, const char* state);
  void stop();

  void update();

  const char* state() const { return overrideUntil_ ? overrideState_ : baseState_; }
  const char* visual() const;
  uint16_t    periodMs() const { return overrideUntil_ ? overridePeriod_ : basePeriod_; }
  uint32_t    remainingMs() const;

 private:
  struct Pattern {
    uint8_t  mode   = LED_OFF;
    uint8_t  bright = 100;
    uint16_t period = 800;
  };

  void write(uint16_t duty);
  uint16_t levelFor(const Pattern& p, uint32_t now) const;

  Pattern     base_, override_;
  const char* baseState_     = "offline";
  const char* overrideState_ = "";
  uint32_t    overrideUntil_ = 0;
  uint32_t    overrideStart_ = 0;
  uint16_t    overridePeriod_ = 0;
  uint16_t    basePeriod_     = 0;
  uint8_t     attachedPin_    = 255;
  bool        attachedInvert_ = false;
  uint16_t    lastDuty_       = 0xFFFF;
};

extern LedEngine led;

// Onboard RGB LED (status only, optional).
void rgbStatus(uint8_t r, uint8_t g, uint8_t b);
