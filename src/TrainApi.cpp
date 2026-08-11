#include "TrainApi.h"
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>

#include "Config.h"

namespace {

const char* UA = "TrainButton/" FW_VERSION " (ESP32)";

// One request, one client — keeps the TLS buffers off the heap between polls.
struct Request {
  WiFiClientSecure secure;
  WiFiClient       plain;
  HTTPClient        http;
  bool             began = false;

  bool open(const ApiConfig& cfg, const String& path) {
    String   url  = cfg.base + path;
    uint16_t toMs = cfg.timeoutSec * 1000;
    http.setTimeout(toMs);
    http.setConnectTimeout(toMs);
    http.setReuse(false);
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
    if (url.startsWith("https://")) {
      // The endpoints are public and the token is the only secret we send; a
      // pinned CA would break on every certificate renewal of the backend.
      secure.setInsecure();
      secure.setTimeout(cfg.timeoutSec);
      began = http.begin(secure, url);
    } else {
      plain.setTimeout(cfg.timeoutSec);
      began = http.begin(plain, url);
    }
    if (began) {
      http.addHeader("X-Train-Token", cfg.token);
      http.addHeader("Accept", "application/json");
      http.setUserAgent(UA);
    }
    return began;
  }

  ~Request() {
    if (began) http.end();
  }
};

String describe(int code) {
  if (code == HTTPC_ERROR_CONNECTION_REFUSED) return "Connection refused";
  if (code == HTTPC_ERROR_CONNECTION_LOST)    return "Connection lost";
  if (code == HTTPC_ERROR_READ_TIMEOUT)       return "Timeout";
  if (code == HTTPC_ERROR_NO_STREAM || code == HTTPC_ERROR_NO_HTTP_SERVER) return "No response";
  if (code == 401 || code == 403)             return "Token rejected";
  if (code == 404)                            return "Endpoint not found";
  if (code >= 500)                            return "Server error " + String(code);
  if (code < 0)                               return "Network error " + String(code);
  return "HTTP " + String(code);
}

String strOrEmpty(JsonVariantConst v) {
  return v.is<const char*>() && v.as<const char*>() ? String(v.as<const char*>()) : String();
}

// Read the body through getString(): the backend answers with
// Transfer-Encoding: chunked, and getStream() hands out the raw framing, which
// ArduinoJson cannot parse. getString() unwraps the chunks for us.
bool readJson(HTTPClient& http, JsonDocument& doc) {
  String body = http.getString();
  DeserializationError err = deserializeJson(doc, body);
  if (err) {
    Serial.printf("[api] cannot parse %d byte body: %s | %.80s\n", body.length(), err.c_str(),
                  body.c_str());
    return false;
  }
  return true;
}

}  // namespace

bool TrainApi::fetchStatus(const ApiConfig& cfg, ApiState& out) {
  out.configured = cfg.token.length() > 0;
  if (!out.configured || WiFi.status() != WL_CONNECTED) {
    out.ok    = false;
    out.error = out.configured ? "No Wi-Fi" : "No token";
    return false;
  }

  Request  req;
  uint32_t t0 = millis();
  if (!req.open(cfg, "/api/train-button/status")) {
    out.ok    = false;
    out.error = "Invalid server address";
    return false;
  }

  int code      = req.http.GET();
  out.latencyMs = millis() - t0;
  out.code      = code;

  if (code != HTTP_CODE_OK) {
    out.ok    = false;
    out.error = describe(code);
    return false;
  }

  JsonDocument doc;
  if (!readJson(req.http, doc)) {
    out.ok    = false;
    out.error = "Unreadable response";
    return false;
  }

  out.username          = strOrEmpty(doc["username"]);
  out.canPress          = doc["canPress"] | false;
  out.secondsUntilReady = doc["secondsUntilReady"] | 0L;
  out.lastUsername      = strOrEmpty(doc["lastUsername"]);
  out.lastPressedAt     = doc["lastPressedAt"] | 0L;
  out.ok                = true;
  out.error             = "";
  out.lastPollMs        = millis();
  return true;
}

PressResult TrainApi::press(const ApiConfig& cfg) {
  PressResult res;
  if (cfg.token.isEmpty()) {
    res.error = "No token stored";
    return res;
  }
  if (WiFi.status() != WL_CONNECTED) {
    res.error = "No Wi-Fi";
    return res;
  }

  Request req;
  if (!req.open(cfg, "/api/train-button/press")) {
    res.error = "Invalid server address";
    return res;
  }
  req.http.addHeader("Content-Type", "application/json");

  int code = req.http.POST("{}");
  res.code = code;

  if (code <= 0) {
    res.error = describe(code);
    return res;
  }

  JsonDocument doc;
  bool         parsed = readJson(req.http, doc);
  if (parsed) {
    res.username         = strOrEmpty(doc["username"]);
    res.secondsRemaining = doc["secondsRemaining"] | 0L;
  }

  if (code == HTTP_CODE_OK) {
    res.ok = parsed ? (doc["success"] | true) : true;
    if (!res.ok) res.error = "Rejected by the server";
  } else if (code == HTTP_CODE_TOO_MANY_REQUESTS) {
    res.locked = true;
    res.error  = "Cooldown running";
  } else {
    res.error = describe(code);
  }
  return res;
}
