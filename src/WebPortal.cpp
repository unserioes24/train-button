#include "WebPortal.h"

#include <ArduinoJson.h>
#include <DNSServer.h>
#include <ESPmDNS.h>
#include <WebServer.h>
#include <WiFi.h>
#include <time.h>

#include "Config.h"
#include "Device.h"
#include "LedEngine.h"
#include "TrainApi.h"
#include "WebPage.h"

static WebServer server(80);
static DNSServer dns;
static bool      dnsRunning = false;

// --------------------------------------------------------------------- helpers
static bool guard() {
  if (settings.uiPass.isEmpty()) return true;
  if (server.authenticate(settings.uiUser.c_str(), settings.uiPass.c_str())) return true;
  server.requestAuthentication();
  return false;
}

static void sendJson(int code, const JsonDocument& doc) {
  String out;
  serializeJson(doc, out);
  server.sendHeader("Cache-Control", "no-store");
  server.send(code, "application/json", out);
}

static void sendOk(bool reboot = false) {
  JsonDocument d;
  d["ok"] = true;
  if (reboot) d["reboot"] = true;
  sendJson(200, d);
}

static void sendErr(int code, const String& msg) {
  JsonDocument d;
  d["error"] = msg;
  sendJson(code, d);
}

static bool readBody(JsonDocument& doc) {
  if (!server.hasArg("plain")) return true;  // empty body is allowed
  return deserializeJson(doc, server.arg("plain")) == DeserializationError::Ok;
}

// --------------------------------------------------------------------- routes
static void handleRoot() {
  if (!guard()) return;
  server.sendHeader("Cache-Control", "no-store");
  server.send_P(200, "text/html; charset=utf-8", WEB_PAGE_HTML);
}

static void handleState() {
  if (!guard()) return;
  DeviceState s   = deviceSnapshot();
  bool        sta = WiFi.status() == WL_CONNECTED;

  JsonDocument d;
  d["setup"] = s.setupMode;

  JsonObject w = d["wifi"].to<JsonObject>();
  w["connected"] = sta;
  w["ssid"]      = s.setupMode ? WiFi.softAPSSID() : settings.ssid;
  w["rssi"]      = sta ? WiFi.RSSI() : 0;
  w["ip"]        = sta ? WiFi.localIP().toString() : WiFi.softAPIP().toString();
  w["host"]      = settings.host;

  JsonObject a = d["api"].to<JsonObject>();
  a["configured"]        = s.api.configured;
  a["ok"]                = s.api.ok;
  a["code"]              = s.api.code;
  a["error"]             = s.api.error;
  a["latency"]           = s.api.latencyMs;
  a["age"]               = s.api.lastPollMs ? (long)((millis() - s.api.lastPollMs) / 1000) : -1;
  a["username"]          = s.api.username;
  a["canPress"]          = s.api.canPress;
  a["secondsUntilReady"] = s.api.secondsUntilReady;
  a["lastUsername"]      = s.api.lastUsername;
  a["lastPressedAt"]     = s.api.lastPressedAt;
  a["lastAgo"] = (deviceTimeSynced() && s.api.lastPressedAt > 0)
                     ? (long)(time(nullptr) - s.api.lastPressedAt)
                     : -1;

  JsonObject l = d["led"].to<JsonObject>();
  l["state"]     = led.state();
  l["visual"]    = led.visual();
  l["periodMs"]  = led.periodMs();
  l["remaining"] = led.remainingMs() / 1000;

  JsonObject p = d["press"].to<JsonObject>();
  p["seq"]              = s.pressSeq;
  p["result"]           = s.pressResult;
  p["error"]            = s.pressError;
  p["secondsRemaining"] = s.pressRemaining;

  JsonObject y = d["sys"].to<JsonObject>();
  y["up"]     = millis() / 1000;
  y["heap"]   = ESP.getFreeHeap();
  y["fw"]     = FW_VERSION;
  y["chip"]   = ESP.getChipModel();
  y["mac"]    = WiFi.macAddress();
  y["reboot"] = s.rebootPending;

  sendJson(200, d);
}

static void handleGetConfig() {
  if (!guard()) return;
  JsonDocument d;
  d["ssid"] = settings.ssid;
  d["host"] = settings.host;
  d["base"] = settings.base;

  // The token itself never leaves the device.
  d["hasToken"] = settings.token.length() > 0;
  d["tokenHint"] = settings.token.length() > 4
                       ? "…" + settings.token.substring(settings.token.length() - 4)
                       : "";

  d["pollSec"]    = settings.pollSec;
  d["timeoutSec"] = settings.timeoutSec;

  d["readyMode"]   = settings.readyMode;
  d["readyBright"] = settings.readyBright;
  d["readyMs"]     = settings.readyMs;
  d["idleMode"]    = settings.idleMode;
  d["idleBright"]  = settings.idleBright;
  d["idleMs"]      = settings.idleMs;
  d["pressSec"]    = settings.pressSec;
  d["pressMs"]     = settings.pressMs;
  d["pressBright"] = settings.pressBright;
  d["errSec"]      = settings.errSec;
  d["errMs"]       = settings.errMs;
  d["errBright"]   = settings.errBright;
  d["instantError"] = settings.instantError;

  d["btnPin"]     = settings.btnPin;
  d["ledPin"]     = settings.ledPin;
  d["rgbPin"]     = settings.rgbPin;
  d["btnPullup"]  = settings.btnPullup;
  d["ledInvert"]  = settings.ledInvert;
  d["rgbOn"]      = settings.rgbOn;
  d["debounceMs"] = settings.debounceMs;
  d["holdSec"]    = settings.holdSec;
  d["resetOn"]    = settings.resetOn;
  d["resetPin"]   = settings.resetPin;
  d["wipeSec"]    = settings.wipeSec;
  d["uiUser"]    = settings.uiUser;
  d["uiLocked"]  = settings.uiPass.length() > 0;
  sendJson(200, d);
}

// Small typed appliers — no generic lambdas, so this also builds on the
// C++11 toolchain of arduino-esp32 2.x.
static bool applyStr(JsonDocument& b, const char* k, String& dst) {
  if (!b[k].is<const char*>()) return false;
  String v = b[k].as<String>();
  v.trim();
  if (v == dst) return false;
  dst = v;
  return true;
}
static bool applyU8(JsonDocument& b, const char* k, uint8_t& dst) {
  if (b[k].isNull()) return false;
  long v = b[k].as<long>();
  if (v < 0) v = 0;
  if (v > 255) v = 255;
  if ((uint8_t)v == dst) return false;
  dst = (uint8_t)v;
  return true;
}
static bool applyU16(JsonDocument& b, const char* k, uint16_t& dst) {
  if (b[k].isNull()) return false;
  long v = b[k].as<long>();
  if (v < 0) v = 0;
  if (v > 65535) v = 65535;
  if ((uint16_t)v == dst) return false;
  dst = (uint16_t)v;
  return true;
}
static bool applyFlag(JsonDocument& b, const char* k, bool& dst) {
  if (!b[k].is<bool>()) return false;
  if (b[k].as<bool>() == dst) return false;
  dst = b[k].as<bool>();
  return true;
}

static void handlePostConfig() {
  if (!guard()) return;
  JsonDocument body;
  if (!readBody(body)) return sendErr(400, "Invalid JSON");

  bool netChanged = false;
  bool hwChanged  = false;

  deviceLock();

  netChanged |= applyStr(body, "ssid", settings.ssid);
  netChanged |= applyStr(body, "pass", settings.pass);
  // The hostname is applied before the next Wi-Fi connect, so saving it during
  // setup must not bounce the device out from under the person configuring it.
  applyStr(body, "host", settings.host);
  applyStr(body, "base", settings.base);
  applyStr(body, "token", settings.token);
  applyStr(body, "uiUser", settings.uiUser);
  if (body["uiPass"].is<const char*>()) settings.uiPass = body["uiPass"].as<String>();
  if (body["clearUiAuth"] | false) settings.uiPass = "";

  applyU16(body, "pollSec", settings.pollSec);
  applyU16(body, "timeoutSec", settings.timeoutSec);
  applyU8(body, "readyMode", settings.readyMode);
  applyU8(body, "readyBright", settings.readyBright);
  applyU16(body, "readyMs", settings.readyMs);
  applyU8(body, "idleMode", settings.idleMode);
  applyU8(body, "idleBright", settings.idleBright);
  applyU16(body, "idleMs", settings.idleMs);
  applyU16(body, "pressSec", settings.pressSec);
  applyU16(body, "pressMs", settings.pressMs);
  applyU8(body, "pressBright", settings.pressBright);
  applyU16(body, "errSec", settings.errSec);
  applyU16(body, "errMs", settings.errMs);
  applyU8(body, "errBright", settings.errBright);
  applyFlag(body, "instantError", settings.instantError);

  hwChanged |= applyU8(body, "btnPin", settings.btnPin);
  hwChanged |= applyU8(body, "ledPin", settings.ledPin);
  hwChanged |= applyU8(body, "rgbPin", settings.rgbPin);
  hwChanged |= applyU16(body, "debounceMs", settings.debounceMs);
  applyU8(body, "holdSec", settings.holdSec);
  applyU8(body, "wipeSec", settings.wipeSec);
  hwChanged |= applyU8(body, "resetPin", settings.resetPin);
  hwChanged |= applyFlag(body, "resetOn", settings.resetOn);
  hwChanged |= applyFlag(body, "btnPullup", settings.btnPullup);
  hwChanged |= applyFlag(body, "ledInvert", settings.ledInvert);
  hwChanged |= applyFlag(body, "rgbOn", settings.rgbOn);

  configSave();
  deviceUnlock();

  if (hwChanged) deviceApplyHardware();
  devicePokePoll();

  sendOk(netChanged);
  if (netChanged) deviceScheduleReboot(1200);
}

static void handleScan() {
  if (!guard()) return;
  int n = WiFi.scanNetworks(false, false);
  JsonDocument d;
  JsonArray    arr = d["networks"].to<JsonArray>();
  for (int i = 0; i < n && i < 25; i++) {
    if (WiFi.SSID(i).isEmpty()) continue;
    JsonObject o = arr.add<JsonObject>();
    o["ssid"] = WiFi.SSID(i);
    o["rssi"] = WiFi.RSSI(i);
    o["lock"] = WiFi.encryptionType(i) != WIFI_AUTH_OPEN;
  }
  WiFi.scanDelete();
  sendJson(200, d);
}

static void handlePress() {
  if (!guard()) return;
  DeviceState s = deviceSnapshot();
  if (settings.instantError && s.api.configured && s.api.ok && !s.api.canPress) {
    led.play(LED_BLINK, settings.errBright, settings.errMs,
             (uint32_t)settings.errSec * 1000UL, "error");
  }
  deviceRequestPress(true);
  JsonDocument d;
  d["queued"] = true;
  d["seq"]    = s.pressSeq;
  sendJson(202, d);
}

static void handleLedTest() {
  if (!guard()) return;
  JsonDocument body;
  if (!readBody(body)) return sendErr(400, "Invalid JSON");
  String p = body["pattern"] | "ready";

  // Preview uses the values from the form, so nothing has to be saved first.
  if (p == "press") {
    led.play(LED_BLINK, body["pressBright"] | settings.pressBright,
             body["pressMs"] | settings.pressMs, 6000, "press");
  } else if (p == "error") {
    led.play(LED_BLINK, body["errBright"] | settings.errBright,
             body["errMs"] | settings.errMs, 4000, "error");
  } else if (p == "idle") {
    led.play(body["idleMode"] | settings.idleMode, body["idleBright"] | settings.idleBright,
             body["idleMs"] | settings.idleMs, 6000, "idle");
  } else {
    led.play(body["readyMode"] | settings.readyMode, body["readyBright"] | settings.readyBright,
             body["readyMs"] | settings.readyMs, 6000, "ready");
  }
  sendOk();
}

static void handleTestConnection() {
  if (!guard()) return;
  ApiState st;
  bool     ok = TrainApi::fetchStatus(deviceApiConfig(), st);
  JsonDocument d;
  d["ok"]       = ok;
  d["code"]     = st.code;
  d["error"]    = st.error;
  d["username"] = st.username;
  d["canPress"] = st.canPress;
  sendJson(200, d);
  devicePokePoll();
}

static void handleReboot() {
  if (!guard()) return;
  sendOk(true);
  deviceScheduleReboot(600);
}

static void handleResetCredentials() {
  if (!guard()) return;
  sendOk(true);
  deviceLock();
  configResetCredentials();
  deviceUnlock();
  deviceScheduleReboot(1200);
}

static void handleFactoryReset() {
  if (!guard()) return;
  sendOk(true);
  deviceFactoryReset();
}

static void handleNotFound() {
  // In setup mode every unknown host lands on the configuration page.
  if (dnsRunning) {
    server.sendHeader("Location", "http://" + WiFi.softAPIP().toString() + "/", true);
    server.send(302, "text/plain", "");
    return;
  }
  server.send(404, "text/plain", "Not found");
}

// --------------------------------------------------------------------- public
void portalBegin() {
  server.on("/", HTTP_GET, handleRoot);
  server.on("/api/state", HTTP_GET, handleState);
  server.on("/api/config", HTTP_GET, handleGetConfig);
  server.on("/api/config", HTTP_POST, handlePostConfig);
  server.on("/api/scan", HTTP_GET, handleScan);
  server.on("/api/press", HTTP_POST, handlePress);
  server.on("/api/led-test", HTTP_POST, handleLedTest);
  server.on("/api/test-connection", HTTP_POST, handleTestConnection);
  server.on("/api/reboot", HTTP_POST, handleReboot);
  server.on("/api/reset-credentials", HTTP_POST, handleResetCredentials);
  server.on("/api/factory-reset", HTTP_POST, handleFactoryReset);
  server.onNotFound(handleNotFound);
  server.begin();

  if (WiFi.getMode() == WIFI_AP) {
    dns.setErrorReplyCode(DNSReplyCode::NoError);
    dns.start(53, "*", WiFi.softAPIP());
    dnsRunning = true;
  } else if (MDNS.begin(settings.host.c_str())) {
    MDNS.addService("http", "tcp", 80);
  }
}

void portalLoop() {
  if (dnsRunning) dns.processNextRequest();
  server.handleClient();
}
