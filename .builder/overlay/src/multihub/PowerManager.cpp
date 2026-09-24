#include "PowerManager.h"

#include <WiFi.h>

#include "PowerStore.h"

namespace PowerManager {

bool autoRefreshOnOpen() {
  PowerConfig config;
  PowerStore::load(config);
  return config.autoRefreshOnOpen;
}

bool radioOffAfterRefresh() {
  PowerConfig config;
  PowerStore::load(config);
  return config.radioOffAfterRefresh;
}

bool turnWifiOffNow() {
  // Arduino-ESP32 API:
  // disconnect(wifioff=true, eraseap=false)
  // We intentionally DO NOT erase saved AP configuration here.
  return WiFi.disconnect(true, false);
}

void afterOnlineOperation() {
  if (!radioOffAfterRefresh()) return;
  turnWifiOffNow();
}

}  // namespace PowerManager
