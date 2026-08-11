#pragma once
#include <Arduino.h>
#include "TrainApi.h"

// What the web UI sees. Filled by the network task, read under a lock.
struct DeviceState {
  ApiState api;
  bool     setupMode      = false;   // access point is up, no station link
  bool     rebootPending  = false;
  uint32_t pressSeq       = 0;       // increments on every finished press
  char     pressResult[8] = "";      // "ok" | "locked" | "error"
  String   pressError;
  long     pressRemaining = 0;
};

DeviceState deviceSnapshot();
ApiConfig   deviceApiConfig();

// Held while Settings are rewritten, so the network task never copies a
// String that is being reassigned. Never call deviceSnapshot() while locked.
void deviceLock();
void deviceUnlock();

// Queues a press. Physical button and web UI use the same path.
void deviceRequestPress(bool fromWeb);

void deviceScheduleReboot(uint32_t inMs);
void deviceFactoryReset();
void deviceApplyHardware();          // re-init button/LED pins after a config change
void devicePokePoll();               // ask the network task to poll now
bool deviceTimeSynced();
