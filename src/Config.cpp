#include "Config.h"
#include <Preferences.h>

Settings settings;

static Preferences prefs;
static const char* NS = "trainbtn";

template <typename T>
static T clampT(T v, T lo, T hi) { return v < lo ? lo : (v > hi ? hi : v); }

void configSanitize() {
  Settings d;  // defaults

  settings.host.trim();
  if (settings.host.isEmpty()) settings.host = d.host;
  settings.host.replace(' ', '-');
  if (settings.host.length() > 30) settings.host = settings.host.substring(0, 30);

  settings.base.trim();
  while (settings.base.endsWith("/")) settings.base.remove(settings.base.length() - 1);
  if (settings.base.isEmpty()) settings.base = d.base;
  if (!settings.base.startsWith("http://") && !settings.base.startsWith("https://"))
    settings.base = "https://" + settings.base;
  settings.pollSec    = clampT<uint16_t>(settings.pollSec, 2, 3600);
  settings.timeoutSec = clampT<uint16_t>(settings.timeoutSec, 2, 30);

  if (settings.readyMode > LED_BLINK) settings.readyMode = d.readyMode;
  if (settings.idleMode  > LED_BLINK) settings.idleMode  = d.idleMode;

  settings.readyBright = clampT<uint8_t>(settings.readyBright, 0, 100);
  settings.idleBright  = clampT<uint8_t>(settings.idleBright, 0, 100);
  settings.pressBright = clampT<uint8_t>(settings.pressBright, 1, 100);
  settings.errBright   = clampT<uint8_t>(settings.errBright, 1, 100);

  settings.readyMs = clampT<uint16_t>(settings.readyMs, 100, 10000);
  settings.idleMs  = clampT<uint16_t>(settings.idleMs, 100, 10000);
  settings.pressMs = clampT<uint16_t>(settings.pressMs, 40, 5000);
  settings.errMs   = clampT<uint16_t>(settings.errMs, 20, 2000);

  settings.pressSec = clampT<uint16_t>(settings.pressSec, 1, 3600);
  settings.errSec   = clampT<uint16_t>(settings.errSec, 1, 300);

  settings.debounceMs = clampT<uint16_t>(settings.debounceMs, 5, 1000);
  settings.holdSec    = clampT<uint8_t>(settings.holdSec, 2, 60);

  if (settings.btnPin > 48) settings.btnPin = d.btnPin;
  if (settings.ledPin > 48) settings.ledPin = d.ledPin;
  if (settings.rgbPin > 48) settings.rgbPin = d.rgbPin;
  if (settings.ledPin == settings.btnPin) settings.ledPin = d.ledPin;
  if (settings.resetPin > 48 || settings.resetPin == settings.btnPin ||
      settings.resetPin == settings.ledPin) settings.resetOn = false;
  settings.wipeSec = clampT<uint8_t>(settings.wipeSec, 2, 60);

  settings.uiUser.trim();
  if (settings.uiUser.isEmpty()) settings.uiUser = d.uiUser;
}

void configLoad() {
  Settings d;
  prefs.begin(NS, true);

  settings.ssid   = prefs.getString("ssid", d.ssid);
  settings.pass   = prefs.getString("pass", d.pass);
  settings.host   = prefs.getString("host", d.host);

  settings.base       = prefs.getString("base", d.base);
  settings.token      = prefs.getString("token", d.token);
  settings.pollSec    = prefs.getUShort("pollSec", d.pollSec);
  settings.timeoutSec = prefs.getUShort("timeoutSec", d.timeoutSec);

  settings.readyMode   = prefs.getUChar("readyMode", d.readyMode);
  settings.readyBright = prefs.getUChar("readyBri", d.readyBright);
  settings.readyMs     = prefs.getUShort("readyMs", d.readyMs);

  settings.idleMode   = prefs.getUChar("idleMode", d.idleMode);
  settings.idleBright = prefs.getUChar("idleBri", d.idleBright);
  settings.idleMs     = prefs.getUShort("idleMs", d.idleMs);

  settings.pressSec    = prefs.getUShort("pressSec", d.pressSec);
  settings.pressMs     = prefs.getUShort("pressMs", d.pressMs);
  settings.pressBright = prefs.getUChar("pressBri", d.pressBright);

  settings.errSec       = prefs.getUShort("errSec", d.errSec);
  settings.errMs        = prefs.getUShort("errMs", d.errMs);
  settings.errBright    = prefs.getUChar("errBri", d.errBright);
  settings.instantError = prefs.getBool("instantErr", d.instantError);

  settings.btnPin     = prefs.getUChar("btnPin", d.btnPin);
  settings.ledPin     = prefs.getUChar("ledPin", d.ledPin);
  settings.rgbPin     = prefs.getUChar("rgbPin", d.rgbPin);
  settings.btnPullup  = prefs.getBool("btnPullup", d.btnPullup);
  settings.ledInvert  = prefs.getBool("ledInvert", d.ledInvert);
  settings.rgbOn      = prefs.getBool("rgbOn", d.rgbOn);
  settings.debounceMs = prefs.getUShort("debounce", d.debounceMs);
  settings.holdSec    = prefs.getUChar("holdSec", d.holdSec);

  settings.resetOn  = prefs.getBool("resetOn", d.resetOn);
  settings.resetPin = prefs.getUChar("resetPin", d.resetPin);
  settings.wipeSec  = prefs.getUChar("wipeSec", d.wipeSec);

  settings.uiUser = prefs.getString("uiUser", d.uiUser);
  settings.uiPass = prefs.getString("uiPass", d.uiPass);

  prefs.end();
  configSanitize();
}

void configSave() {
  configSanitize();
  prefs.begin(NS, false);

  prefs.putString("ssid", settings.ssid);
  prefs.putString("pass", settings.pass);
  prefs.putString("host", settings.host);

  prefs.putString("base", settings.base);
  prefs.putString("token", settings.token);
  prefs.putUShort("pollSec", settings.pollSec);
  prefs.putUShort("timeoutSec", settings.timeoutSec);

  prefs.putUChar("readyMode", settings.readyMode);
  prefs.putUChar("readyBri", settings.readyBright);
  prefs.putUShort("readyMs", settings.readyMs);

  prefs.putUChar("idleMode", settings.idleMode);
  prefs.putUChar("idleBri", settings.idleBright);
  prefs.putUShort("idleMs", settings.idleMs);

  prefs.putUShort("pressSec", settings.pressSec);
  prefs.putUShort("pressMs", settings.pressMs);
  prefs.putUChar("pressBri", settings.pressBright);

  prefs.putUShort("errSec", settings.errSec);
  prefs.putUShort("errMs", settings.errMs);
  prefs.putUChar("errBri", settings.errBright);
  prefs.putBool("instantErr", settings.instantError);

  prefs.putUChar("btnPin", settings.btnPin);
  prefs.putUChar("ledPin", settings.ledPin);
  prefs.putUChar("rgbPin", settings.rgbPin);
  prefs.putBool("btnPullup", settings.btnPullup);
  prefs.putBool("ledInvert", settings.ledInvert);
  prefs.putBool("rgbOn", settings.rgbOn);
  prefs.putUShort("debounce", settings.debounceMs);
  prefs.putUChar("holdSec", settings.holdSec);

  prefs.putBool("resetOn", settings.resetOn);
  prefs.putUChar("resetPin", settings.resetPin);
  prefs.putUChar("wipeSec", settings.wipeSec);

  prefs.putString("uiUser", settings.uiUser);
  prefs.putString("uiPass", settings.uiPass);

  prefs.end();
}

void configResetCredentials() {
  Settings d;
  settings.ssid   = d.ssid;
  settings.pass   = d.pass;
  settings.token  = d.token;
  settings.base   = d.base;
  settings.uiPass = d.uiPass;
  settings.uiUser = d.uiUser;
  configSave();
}

void configFactoryReset() {
  prefs.begin(NS, false);
  prefs.clear();
  prefs.end();
  settings = Settings();
}
