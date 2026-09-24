#include "TimeStore.h"

#include <ArduinoJson.h>
#include <HalStorage.h>

#include <string>

namespace {
constexpr const char* MOD = "Time";

struct TimezonePreset {
  const char* name;
  const char* rule;
};

// POSIX TZ rules. They are deliberately explicit instead of assuming a locale.
// The user selects the zone in MultiHub settings.
constexpr TimezonePreset PRESETS[TimeStore::PRESET_COUNT] = {
    {"UTC", "UTC0"},
    {"Polska", "CET-1CEST,M3.5.0,M10.5.0/3"},
    {"UK", "GMT0BST,M3.5.0/1,M10.5.0"},
    {"US Eastern", "EST5EDT,M3.2.0/2,M11.1.0/2"},
};

bool ensureRoot() {
  if (!Storage.ensureDirectoryExists("/.x4-multihub")) return false;
  return Storage.ensureDirectoryExists(TimeStore::ROOT);
}
}  // namespace

namespace TimeStore {

TimeConfig defaults() {
  return {};
}

uint8_t normalizedPreset(const uint8_t index) {
  return index < PRESET_COUNT ? index : 0;
}

const char* presetName(const uint8_t index) {
  return PRESETS[normalizedPreset(index)].name;
}

const char* presetRule(const uint8_t index) {
  return PRESETS[normalizedPreset(index)].rule;
}

bool load(TimeConfig& config) {
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

  config.timezonePreset =
      normalizedPreset(static_cast<uint8_t>(doc["timezone_preset"] | 0));
  config.autoToday = doc["auto_today"] | false;
  config.lastSyncEpoch = doc["last_sync_epoch"] | 0LL;
  return true;
}

bool save(const TimeConfig& config) {
  if (!ensureRoot()) return false;

  JsonDocument doc;
  doc["timezone_preset"] = normalizedPreset(config.timezonePreset);
  doc["auto_today"] = config.autoToday;
  doc["last_sync_epoch"] = config.lastSyncEpoch;

  String raw;
  serializeJsonPretty(doc, raw);

  const std::string temp = std::string(CONFIG_PATH) + ".tmp";
  if (!Storage.writeFile(temp.c_str(), raw)) return false;

  Storage.remove(CONFIG_PATH);
  if (!Storage.rename(temp.c_str(), CONFIG_PATH)) {
    Storage.remove(temp.c_str());
    return false;
  }
  return true;
}

}  // namespace TimeStore
