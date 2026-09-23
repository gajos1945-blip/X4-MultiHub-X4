#include "WeatherStore.h"

#include <ArduinoJson.h>
#include <HalStorage.h>

namespace {
constexpr const char* MOD = "Weather";

bool ensureRoot() {
  if (!Storage.ensureDirectoryExists("/.x4-multihub")) return false;
  return Storage.ensureDirectoryExists(WeatherStore::ROOT);
}
}  // namespace

namespace WeatherStore {

bool validCity(const std::string& city) {
  return !city.empty() && city.size() <= 96 &&
         city.find('\n') == std::string::npos &&
         city.find('\r') == std::string::npos;
}

bool loadCity(std::string& city) {
  city.clear();

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
  const char* value = doc["city"] | nullptr;
  if (!value || !validCity(value)) return false;
  city = value;
  return true;
}

bool saveCity(const std::string& city) {
  if (!validCity(city) || !ensureRoot()) return false;

  JsonDocument doc;
  doc["city"] = city;
  String raw;
  serializeJsonPretty(doc, raw);
  return Storage.writeFile(CONFIG_PATH, raw);
}

}  // namespace WeatherStore
