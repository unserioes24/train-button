// Train Button — an ESP32 push button that starts the model train on
// unserioes24's stream, with a web interface for WiFi, token and LED behaviour.
#include <Arduino.h>
#include <WiFi.h>
#include <time.h>

#include "Config.h"
#include "Device.h"
#include "LedEngine.h"
#include "TrainApi.h"
#include "WebPortal.h"

// ---------------------------------------------------------------- shared state
static SemaphoreHandle_t stateLock = nullptr;
static DeviceState       state;
static QueueHandle_t     pressQueue = nullptr;
static volatile bool     pollNow    = true;
static volatile uint32_t rebootAt   = 0;
static volatile bool     factoryWipe = false;

// LED work handed from the network task to the loop.
enum PendingLed : uint8_t { PEND_NONE = 0, PEND_CONFIRM, PEND_ERROR };
static volatile uint8_t pendingLed = PEND_NONE;

#define LOCK()   xSemaphoreTake(stateLock, portMAX_DELAY)
#define UNLOCK() xSemaphoreGive(stateLock)

DeviceState deviceSnapshot() {
  LOCK();
  DeviceState copy = state;
  UNLOCK();
  return copy;
}

ApiConfig deviceApiConfig() {
  LOCK();
  ApiConfig c;
  c.base       = settings.base;
  c.token      = settings.token;
  c.timeoutSec = settings.timeoutSec;
  UNLOCK();
  return c;
}

void deviceLock() { LOCK(); }
void deviceUnlock() { UNLOCK(); }

void devicePokePoll() { pollNow = true; }

void deviceScheduleReboot(uint32_t inMs) {
  rebootAt = millis() + inMs;
  if (rebootAt == 0) rebootAt = 1;
  LOCK();
  state.rebootPending = true;
  UNLOCK();
}

void deviceFactoryReset() {
  factoryWipe = true;
  deviceScheduleReboot(1200);
}

bool deviceTimeSynced() {
  return time(nullptr) > 1700000000;  // NTP has run
}

// ---------------------------------------------------------------- press queue
void deviceRequestPress(bool fromWeb) {
  uint8_t msg = fromWeb ? 1 : 0;
  if (pressQueue) xQueueSend(pressQueue, &msg, 0);
}

// ---------------------------------------------------------------- LED states
static void ledShowBase() {
  DeviceState s = deviceSnapshot();

  if (s.setupMode) {
    // SOS: unmistakable "I need to be set up", even across the room.
    led.setBase(LED_SOS, 100, SOS_CYCLE_MS, "setup");
    rgbStatus(60, 30, 0);
    return;
  }
  if (WiFi.status() != WL_CONNECTED) {
    led.setBase(LED_BLINK, 30, 1600, "offline");
    rgbStatus(40, 0, 0);
    return;
  }
  if (!s.api.configured || !s.api.ok) {
    led.setBase(LED_BLINK, 25, 2400, "offline");
    rgbStatus(35, 15, 0);
    return;
  }
  if (s.api.canPress) {
    led.setBase(settings.readyMode, settings.readyBright, settings.readyMs, "ready");
    rgbStatus(0, 30, 22);
  } else {
    led.setBase(settings.idleMode, settings.idleBright, settings.idleMs, "idle");
    rgbStatus(18, 4, 26);
  }
}

static void ledConfirm() {
  led.play(LED_BLINK, settings.pressBright, settings.pressMs,
           (uint32_t)settings.pressSec * 1000UL, "press");
  rgbStatus(0, 40, 12);
}

static void ledError() {
  led.play(LED_BLINK, settings.errBright, settings.errMs,
           (uint32_t)settings.errSec * 1000UL, "error");
  rgbStatus(60, 0, 8);
}

// ---------------------------------------------------------------- buttons
struct Btn {
  uint8_t  pin       = 255;
  bool     pullup    = true;
  bool     enabled   = false;
  bool     stable    = false;  // true = pressed
  bool     raw       = false;
  uint32_t changedAt = 0;
  uint32_t pressedAt = 0;
  bool     handled   = false;  // a hold action already fired for this press

  void attach(uint8_t p, bool pu, bool en) {
    pin = p; pullup = pu; enabled = en;
    if (enabled) pinMode(pin, pullup ? INPUT_PULLUP : INPUT_PULLDOWN);
    stable = raw = handled = false;
  }
  bool down() const { return digitalRead(pin) == (pullup ? LOW : HIGH); }
  uint32_t heldMs(uint32_t now) const { return (enabled && stable) ? now - pressedAt : 0; }
};

static Btn mainBtn, resetBtn;

static void buttonsBegin() {
  mainBtn.attach(PIN_BUTTON, settings.btnPullup, true);
  resetBtn.attach(PIN_RESET, settings.btnPullup, settings.resetOn);
}

void deviceApplyHardware() {
  buttonsBegin();
  led.applyPins();
}

// 0 = nothing, 1 = pressed, 2 = released
static uint8_t pollEdge(Btn& b, uint32_t now) {
  if (!b.enabled) return 0;
  bool level = b.down();
  if (level != b.raw) {
    b.raw       = level;
    b.changedAt = now;
  }
  if (b.raw != b.stable && (now - b.changedAt) >= settings.debounceMs) {
    b.stable = b.raw;
    if (b.stable) {
      b.pressedAt = now;
      b.handled   = false;
      return 1;
    }
    return 2;
  }
  return 0;
}

static void onPressed() {
  DeviceState s = deviceSnapshot();
  bool locked   = s.api.configured && s.api.ok && !s.api.canPress;

  if (settings.instantError && locked) {
    ledError();                    // no waiting for the round trip
  } else if (!locked) {
    ledConfirm();                  // optimistic; a 429 replaces it below
  }
  deviceRequestPress(false);
}

// Clears the Wi-Fi credentials and comes back up in setup mode.
static void reopenSetup() {
  led.play(LED_BLINK, 100, 120, 1500, "error");
  LOCK();
  settings.ssid = "";
  settings.pass = "";
  configSave();
  UNLOCK();
  deviceScheduleReboot(1600);
}

static void buttonsLoop() {
  if (mainBtn.pullup != settings.btnPullup || resetBtn.enabled != settings.resetOn) {
    buttonsBegin();
  }
  uint32_t now = millis();

  if (pollEdge(mainBtn, now) == 1) onPressed();
  if (!mainBtn.handled && mainBtn.heldMs(now) >= (uint32_t)settings.holdSec * 1000UL) {
    mainBtn.handled = true;
    Serial.println("[btn] long press - reopening setup");
    reopenSetup();
  }

  // The reset button drops Wi-Fi, password, token and server address.
  pollEdge(resetBtn, now);
  if (!resetBtn.handled && resetBtn.heldMs(now) >= (uint32_t)settings.wipeSec * 1000UL) {
    resetBtn.handled = true;
    Serial.println("[btn] reset held - clearing credentials");
    led.play(LED_BLINK, 100, 90, 2500, "error");
    LOCK();
    configResetCredentials();
    UNLOCK();
    deviceScheduleReboot(2000);
  }
}

// ---------------------------------------------------------------- wifi
static uint32_t wifiRetryAt = 0;

// These boards have a small ceramic antenna. In a crowded 2.4 GHz band a weak
// access point simply drowns, so pick the quietest of the three non-overlapping
// channels instead of always landing on 1.
static uint8_t quietestChannel() {
  int n = WiFi.scanNetworks(false, true);
  if (n <= 0) return 1;

  int load[12] = {0};
  for (int i = 0; i < n; i++) {
    int ch = WiFi.channel(i);
    if (ch < 1 || ch > 11) continue;
    // A network bleeds into the two channels either side of its own.
    for (int c = ch - 2; c <= ch + 2; c++)
      if (c >= 1 && c <= 11) load[c] += (c == ch) ? 3 : 1;
  }
  WiFi.scanDelete();

  uint8_t best = 1;
  for (uint8_t c : {1, 6, 11})
    if (load[c] < load[best]) best = c;
  Serial.printf("[wifi] channel load 1=%d 6=%d 11=%d -> using %d\n", load[1], load[6], load[11], best);
  return best;
}

static void startSetupMode() {
  WiFi.persistent(false);
  WiFi.disconnect(true, true);
  WiFi.mode(WIFI_STA);
  delay(100);
  uint8_t ch = quietestChannel();

  WiFi.mode(WIFI_AP);
  delay(100);
  WiFi.setTxPower(WIFI_POWER_19_5dBm);  // some of these boards ship very quiet

  // Read the eFuse MAC — WiFi.macAddress() is still all zeroes this early.
  uint64_t chip = ESP.getEfuseMac();
  char     ap[40];
  snprintf(ap, sizeof(ap), "TrainButton-%02X%02X",
           (uint8_t)(chip >> 32), (uint8_t)(chip >> 40));

  // No softAPConfig() here: 192.168.4.1/24 is the default anyway, and calling it
  // before softAP() has been seen to leave the DHCP server down on core 3.x —
  // which looks exactly like "it will not connect" from a phone.
  // Open and visible: the access point only lives until Wi-Fi is configured,
  // and a password nobody can look up would just lock you out.
  bool ok = WiFi.softAP(ap, nullptr, ch, 0, 4);

  WiFi.onEvent([](WiFiEvent_t, WiFiEventInfo_t) {
    Serial.printf("[wifi] a device joined the setup network (%d connected)\n",
                  WiFi.softAPgetStationNum());
  }, ARDUINO_EVENT_WIFI_AP_STACONNECTED);

  // Association is not the same as a usable connection — this is the event that
  // proves DHCP handed out an address.
  WiFi.onEvent([](WiFiEvent_t, WiFiEventInfo_t info) {
    Serial.printf("[wifi] handed out %s\n",
                  IPAddress(info.wifi_ap_staipassigned.ip.addr).toString().c_str());
  }, ARDUINO_EVENT_WIFI_AP_STAIPASSIGNED);

  LOCK();
  state.setupMode = true;
  UNLOCK();

  Serial.printf("[wifi] setup mode - join \"%s\" (open), then open http://%s\n", ap,
                WiFi.softAPIP().toString().c_str());
  Serial.printf("[wifi] softAP=%s channel=%d mac=%s txpower=%d\n", ok ? "up" : "FAILED",
                WiFi.channel(), WiFi.softAPmacAddress().c_str(), (int)WiFi.getTxPower());
}

static bool startStation(uint32_t waitMs) {
  WiFi.mode(WIFI_STA);
  WiFi.setSleep(false);
  WiFi.setHostname(DEVICE_HOST);
  WiFi.begin(settings.ssid.c_str(), settings.pass.c_str());

  uint32_t t0 = millis();
  while (millis() - t0 < waitMs) {
    if (WiFi.status() == WL_CONNECTED) {
      LOCK();
      state.setupMode = false;
      UNLOCK();
      Serial.printf("[wifi] connected, http://%s.local (%s)\n",
                    DEVICE_HOST, WiFi.localIP().toString().c_str());
      configTime(0, 0, "pool.ntp.org", "time.cloudflare.com");
      return true;
    }
    led.update();
    delay(20);
  }
  return false;
}

static void wifiLoop() {
  DeviceState s = deviceSnapshot();
  if (s.setupMode || settings.ssid.isEmpty()) return;
  if (WiFi.status() == WL_CONNECTED) {
    wifiRetryAt = 0;
    return;
  }
  uint32_t now = millis();
  if (wifiRetryAt == 0) {
    wifiRetryAt = now + 8000;
    return;
  }
  if ((int32_t)(now - wifiRetryAt) >= 0) {
    Serial.println("[wifi] reconnecting");
    WiFi.disconnect();
    WiFi.begin(settings.ssid.c_str(), settings.pass.c_str());
    wifiRetryAt = now + 20000;
  }
}

// ---------------------------------------------------------------- network task
static void netTask(void*) {
  uint32_t nextPoll = 0;
  for (;;) {
    uint8_t msg;
    if (pressQueue && xQueueReceive(pressQueue, &msg, 0) == pdTRUE) {
      PressResult r = TrainApi::press(deviceApiConfig());

      LOCK();
      state.pressSeq++;
      state.pressError     = r.error;
      state.pressRemaining = r.secondsRemaining;
      strncpy(state.pressResult, r.ok ? "ok" : (r.locked ? "locked" : "error"),
              sizeof(state.pressResult) - 1);
      if (r.ok) {
        state.api.canPress          = false;
        state.api.secondsUntilReady = r.secondsRemaining > 0 ? r.secondsRemaining : 3600;
        if (r.username.length()) state.api.lastUsername = r.username;
      } else if (r.locked) {
        state.api.canPress          = false;
        state.api.secondsUntilReady = r.secondsRemaining;
      }
      UNLOCK();

      pendingLed = r.ok ? PEND_CONFIRM : PEND_ERROR;
      nextPoll   = millis() + 1500;
      Serial.printf("[press] %s (%d)\n", r.ok ? "ok" : r.error.c_str(), r.code);
      continue;
    }

    if (WiFi.status() == WL_CONNECTED && (pollNow || (int32_t)(millis() - nextPoll) >= 0)) {
      pollNow = false;
      ApiConfig cfg = deviceApiConfig();
      ApiState  tmp = deviceSnapshot().api;
      TrainApi::fetchStatus(cfg, tmp);

      LOCK();
      // Keep a successful press visible until the server confirms it.
      state.api = tmp;
      UNLOCK();

      nextPoll = millis() + (uint32_t)settings.pollSec * 1000UL;
    }
    vTaskDelay(pdMS_TO_TICKS(40));
  }
}

// ---------------------------------------------------------------- setup / loop
void setup() {
  Serial.begin(115200);
  delay(200);
  Serial.println("\n[boot] Train Button " FW_VERSION);

  stateLock  = xSemaphoreCreateMutex();
  pressQueue = xQueueCreate(4, sizeof(uint8_t));

  configLoad();
  led.begin();
  buttonsBegin();
  rgbStatus(20, 10, 40);

  led.setBase(LED_BREATHE, 40, 1200, "offline");

  if (settings.ssid.isEmpty() || !startStation(20000)) startSetupMode();

  portalBegin();
  xTaskCreatePinnedToCore(netTask, "net", 12288, nullptr, 1, nullptr, 0);
}

void loop() {
  buttonsLoop();
  wifiLoop();
  portalLoop();

  uint8_t pend = pendingLed;
  if (pend != PEND_NONE) {
    pendingLed = PEND_NONE;
    if (pend == PEND_CONFIRM) ledConfirm();
    else                      ledError();
  }

  static uint32_t nextBase = 0;
  if ((int32_t)(millis() - nextBase) >= 0) {
    nextBase = millis() + 200;
    ledShowBase();
  }
  led.update();

  if (rebootAt && (int32_t)(millis() - rebootAt) >= 0) {
    if (factoryWipe) configFactoryReset();
    Serial.println("[sys] restarting");
    delay(120);
    ESP.restart();
  }
  delay(2);
}
