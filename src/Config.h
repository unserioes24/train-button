#pragma once
#include <Arduino.h>

#define FW_VERSION "1.0.0"

// LED pattern modes, shared between config and the LED engine.
enum LedMode : uint8_t {
  LED_OFF     = 0,
  LED_SOLID   = 1,
  LED_BREATHE = 2,
  LED_BLINK   = 3
};

struct Settings {
  // --- network ---
  String   ssid;
  String   pass;
  String   host     = "trainbutton";   // mDNS name + AP suffix

  // --- backend ---
  String   base     = "https://data.unserioes24.de";
  String   token;                       // never leaves the device in clear text
  uint16_t pollSec    = 5;
  uint16_t timeoutSec = 8;

  // --- LED behaviour ---
  uint8_t  readyMode   = LED_SOLID;   // train may be started
  uint8_t  readyBright = 100;
  uint16_t readyMs     = 800;

  uint8_t  idleMode    = LED_OFF;     // cooldown running
  uint8_t  idleBright  = 8;
  uint16_t idleMs      = 3000;

  uint16_t pressSec    = 120;         // confirmation blink after a successful press
  uint16_t pressMs     = 500;
  uint8_t  pressBright = 100;

  uint16_t errSec      = 3;           // fast error blink
  uint16_t errMs       = 80;
  uint8_t  errBright   = 100;
  bool     instantError = true;       // blink before the server answers

  // --- hardware ---
  uint8_t  btnPin     = 4;            // button switch
  uint8_t  ledPin     = 5;            // single-colour LED ring in the button
  uint8_t  rgbPin     = 48;           // the board's own WS2812, if it has one
  bool     btnPullup  = true;         // button shorts to GND
  bool     ledInvert  = false;
  bool     rgbOn      = false;        // off by default — not every board has one
  uint16_t debounceMs = 40;
  uint8_t  holdSec    = 5;            // hold the main button this long to force setup mode

  // Optional second button, wired the same way as the main one.
  bool     resetOn  = false;
  uint8_t  resetPin = 6;
  uint8_t  wipeSec  = 8;              // hold the reset button this long for a factory reset

  // --- web UI access ---
  String   uiUser = "admin";
  String   uiPass;                     // empty = no login required
};

extern Settings settings;

void configLoad();
void configSave();

// Clears Wi-Fi network and password, the backend token, the server address and
// the web UI password. LED patterns and pin assignments survive.
void configResetCredentials();

// Wipes the whole NVS namespace, including LED and hardware settings.
void configFactoryReset();

// Clamps every numeric field into a range the firmware can actually run with.
void configSanitize();
