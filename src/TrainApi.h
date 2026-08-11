#pragma once
#include <Arduino.h>

// Snapshot of the settings a request needs. Copied under the state lock so the
// network task never reads Settings while the web handler rewrites them.
struct ApiConfig {
  String   base;
  String   token;
  uint16_t timeoutSec = 8;
};

// Mirror of GET /api/train-button/status
struct ApiState {
  bool     configured        = false;  // a token is stored
  bool     ok                = false;  // last request succeeded
  int      code              = 0;      // last HTTP status code
  String   error;                      // last error, empty when ok
  uint32_t latencyMs         = 0;
  uint32_t lastPollMs        = 0;      // millis() of the last successful poll
  String   username;
  bool     canPress          = false;
  long     secondsUntilReady = 0;
  String   lastUsername;
  long     lastPressedAt     = 0;      // unix seconds, 0 = unknown
};

// Result of POST /api/train-button/press
struct PressResult {
  bool   ok               = false;
  bool   locked           = false;  // HTTP 429, cooldown still running
  int    code             = 0;
  String error;
  long   secondsRemaining = 0;
  String username;
};

namespace TrainApi {
bool        fetchStatus(const ApiConfig& cfg, ApiState& out);
PressResult press(const ApiConfig& cfg);
}  // namespace TrainApi
