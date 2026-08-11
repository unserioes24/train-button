#pragma once
#include <Arduino.h>

#define FW_VERSION "1.0.0"

// The device answers at http://trainbutton.local once it is on your network.
#define DEVICE_HOST "trainbutton"

// Fixed wiring. Not configurable at runtime on purpose — a wrong pin typed into
// a browser can only ever break the device. See README.md.
#define PIN_BUTTON 4   // switch, other leg to GND
#define PIN_LED    5   // LED ring, through a series resistor to GND
#define PIN_RESET  6   // optional second switch, other leg to GND
#define PIN_RGB    48  // the board's own WS2812, if it has one

// LED pattern modes, shared between config and the LED engine.
// LED_SOS is used by the firmware only — it is not offered in the web interface.
enum LedMode : uint8_t {
  LED_OFF     = 0,
  LED_SOLID   = 1,
  LED_BREATHE = 2,
  LED_BLINK   = 3,
  LED_SOS     = 4
};

// One full "... --- ..." takes this long.
#define SOS_CYCLE_MS 6800

struct Settings {
  // --- network ---
  String   ssid;
  String   pass;

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

  // --- hardware behaviour (the pins themselves are fixed above) ---
  bool     btnPullup  = true;         // switch shorts to GND
  bool     ledInvert  = false;
  bool     rgbOn      = false;        // off by default — not every board has one
  uint16_t debounceMs = 40;
  uint8_t  holdSec    = 5;            // hold the main button this long for setup mode

  bool     resetOn = false;           // an optional second button is wired up
  uint8_t  wipeSec = 8;               // hold it this long to clear the credentials
};

extern Settings settings;

void configLoad();
void configSave();

// Clears Wi-Fi network and password, the backend token and the server address.
// LED patterns and hardware behaviour survive.
void configResetCredentials();

// Wipes the whole NVS namespace.
void configFactoryReset();

// Clamps every numeric field into a range the firmware can actually run with.
void configSanitize();
