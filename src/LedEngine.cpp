#include "LedEngine.h"
#include <math.h>

LedEngine led;

static const uint16_t PWM_MAX  = 4095;   // 12 bit
static const uint32_t PWM_FREQ = 5000;
static const uint8_t  PWM_CH   = 0;

void LedEngine::begin() {
  applyPins();
}

void LedEngine::applyPins() {
  if (attachedPin_ != 255 && attachedPin_ != settings.ledPin) {
#if ESP_ARDUINO_VERSION_MAJOR >= 3
    ledcDetach(attachedPin_);
#else
    ledcDetachPin(attachedPin_);
#endif
  }
  attachedPin_    = settings.ledPin;
  attachedInvert_ = settings.ledInvert;
  lastDuty_       = 0xFFFF;

#if ESP_ARDUINO_VERSION_MAJOR >= 3
  ledcAttach(attachedPin_, PWM_FREQ, 12);
#else
  ledcSetup(PWM_CH, PWM_FREQ, 12);
  ledcAttachPin(attachedPin_, PWM_CH);
#endif
  write(0);
}

void LedEngine::write(uint16_t duty) {
  if (duty > PWM_MAX) duty = PWM_MAX;
  uint16_t out = attachedInvert_ ? (PWM_MAX - duty) : duty;
  if (out == lastDuty_) return;
  lastDuty_ = out;
#if ESP_ARDUINO_VERSION_MAJOR >= 3
  ledcWrite(attachedPin_, out);
#else
  ledcWrite(PWM_CH, out);
#endif
}

void LedEngine::setBase(uint8_t mode, uint8_t brightPct, uint16_t periodMs, const char* state) {
  base_.mode   = mode;
  base_.bright = brightPct;
  base_.period = periodMs ? periodMs : 800;
  basePeriod_  = base_.period;
  baseState_   = state;
}

void LedEngine::play(uint8_t mode, uint8_t brightPct, uint16_t periodMs, uint32_t durationMs, const char* state) {
  override_.mode   = mode;
  override_.bright = brightPct;
  override_.period = periodMs ? periodMs : 200;
  overridePeriod_  = override_.period;
  overrideState_   = state;
  overrideStart_   = millis();
  overrideUntil_   = overrideStart_ + durationMs;
  if (overrideUntil_ == 0) overrideUntil_ = 1;  // never land on "inactive"
}

void LedEngine::stop() {
  overrideUntil_ = 0;
}

uint32_t LedEngine::remainingMs() const {
  if (!overrideUntil_) return 0;
  uint32_t now = millis();
  return (int32_t)(overrideUntil_ - now) > 0 ? overrideUntil_ - now : 0;
}

// Perceived brightness: the eye is not linear, so square the duty cycle.
static uint16_t gamma16(uint8_t pct, float factor) {
  float f = (pct / 100.0f) * factor;
  if (f <= 0.0f) return 0;
  if (f > 1.0f) f = 1.0f;
  return (uint16_t)(f * f * PWM_MAX + 0.5f);
}

uint16_t LedEngine::levelFor(const Pattern& p, uint32_t now) const {
  switch (p.mode) {
    case LED_SOLID:
      return gamma16(p.bright, 1.0f);
    case LED_BLINK:
      return (now % p.period) < (uint32_t)(p.period / 2) ? gamma16(p.bright, 1.0f) : 0;
    case LED_BREATHE: {
      float phase = (float)(now % p.period) / (float)p.period;
      float wave  = (1.0f - cosf(phase * 2.0f * (float)PI)) * 0.5f;
      return gamma16(p.bright, 0.06f + 0.94f * wave);
    }
    case LED_OFF:
    default:
      return 0;
  }
}

void LedEngine::update() {
  if (attachedPin_ != settings.ledPin || attachedInvert_ != settings.ledInvert) applyPins();

  uint32_t now = millis();
  if (overrideUntil_ && (int32_t)(now - overrideUntil_) >= 0) overrideUntil_ = 0;

  const Pattern& p = overrideUntil_ ? override_ : base_;
  // Restart the phase when an override begins so a blink always starts lit.
  uint32_t t = overrideUntil_ ? (now - overrideStart_) : now;
  write(levelFor(p, t));
}

const char* LedEngine::visual() const {
  uint8_t m = overrideUntil_ ? override_.mode : base_.mode;
  switch (m) {
    case LED_SOLID:   return "solid";
    case LED_BREATHE: return "breathe";
    case LED_BLINK:   return "blink";
    default:          return "off";
  }
}

static void rgbWrite(uint8_t pin, uint8_t r, uint8_t g, uint8_t b) {
#if ESP_ARDUINO_VERSION_MAJOR >= 3
  rgbLedWrite(pin, r, g, b);
#else
  neopixelWrite(pin, r, g, b);
#endif
}

void rgbStatus(uint8_t r, uint8_t g, uint8_t b) {
  static bool    inited = false;
  static uint8_t lr = 255, lg = 255, lb = 255;
  static uint8_t pin = 255;
  if (!settings.rgbOn) {
    if (inited) { rgbWrite(pin, 0, 0, 0); inited = false; }
    return;
  }
  if (!inited || pin != settings.rgbPin) {
    pin    = settings.rgbPin;
    inited = true;
    lr = lg = lb = 255;
  }
  if (r == lr && g == lg && b == lb) return;
  lr = r; lg = g; lb = b;
  neopixelWrite(pin, r, g, b);
}
