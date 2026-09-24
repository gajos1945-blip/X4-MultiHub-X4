#pragma once

namespace PowerManager {

bool autoRefreshOnOpen();
bool radioOffAfterRefresh();

// Turns the station radio off without asking Arduino-ESP32 to erase AP config.
// Returns WiFi.disconnect(wifioff=true, eraseap=false).
bool turnWifiOffNow();

// Called by MultiHub activities only after their complete network operation/batch.
// No action unless the user explicitly enabled radioOffAfterRefresh.
void afterOnlineOperation();

}  // namespace PowerManager
