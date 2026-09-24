#include "PowerStore.h"

#include <ArduinoJson.h>
#include <HalStorage.h>

#include <string>

namespace {
constexpr const char* MOD = "Power";

bool ensureRoot() {
  if (!Storage.ensureDirectoryExists("/.x4-multihub")) return false;
  return Storage.ensureDirectoryExists(PowerStore::ROOT);
}
}

namespace PowerStore {

PowerConfig defaults() {
  PowerConfig config;
  config.autoRefreshOnOpen = false;
  config.radioOffAfterRefresh = false;
  return config;
}

bool load(PowerConfig& config) {
  config = defaults();

  HalFile file;
  if (!Storage.openFileForRead(MOD, CONFIG_PATH, file)) return false;

  std::string raw;
  raw.reserve(256);
  while (file.available() && raw.size() < 2048) {
    const int ch = file.read();
    if (ch < 0) break;
    raw.push_back(static_cast<char>(ch));
  }
  file.close();

  JsonDocument doc;
  if (deserializeJson(doc, raw)) return false;

  config.autoRefreshOnOpen = doc["auto_refresh_on_open"] | false;
  config.radioOffAfterRefresh = doc["radio_off_after_refresh"] | false;
  return true;
}

bool save(const PowerConfig& config) {
  if (!ensureRoot()) return false;

  JsonDocument doc;
  doc["auto_refresh_on_open"] = config.autoRefreshOnOpen;
  doc["radio_off_after_refresh"] = config.radioOffAfterRefresh;

  String raw;
  serializeJsonPretty(doc, raw);
  return Storage.writeFile(CONFIG_PATH, raw);
}

}  // namespace PowerStore
