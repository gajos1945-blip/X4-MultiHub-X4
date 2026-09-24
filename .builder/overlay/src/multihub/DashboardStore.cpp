#include "DashboardStore.h"

#include <ArduinoJson.h>
#include <HalStorage.h>

#include <algorithm>

namespace {
constexpr const char* MOD = "Dashboard";

bool ensureRoot() {
  if (!Storage.ensureDirectoryExists("/.x4-multihub")) return false;
  return Storage.ensureDirectoryExists(DashboardStore::ROOT);
}

bool hasCard(const std::vector<std::string>& values, const std::string& card) {
  return std::find(values.begin(), values.end(), card) != values.end();
}
}  // namespace

namespace DashboardStore {

bool validCard(const std::string& card) {
  return card == "reader" || card == "weather" || card == "markets" || card == "planner";
}

DashboardConfig defaults() {
  DashboardConfig config;
  config.order = {"reader", "weather", "markets", "planner"};
  return config;
}

bool load(DashboardConfig& config) {
  config = defaults();

  HalFile file;
  if (!Storage.openFileForRead(MOD, CONFIG_PATH, file)) return false;

  std::string raw;
  raw.reserve(512);
  while (file.available() && raw.size() < 4096) {
    const int ch = file.read();
    if (ch < 0) break;
    raw.push_back(static_cast<char>(ch));
  }
  file.close();

  JsonDocument doc;
  if (deserializeJson(doc, raw)) return false;

  config.readerVisible = doc["reader_visible"] | true;
  config.weatherVisible = doc["weather_visible"] | true;
  config.marketsVisible = doc["markets_visible"] | true;
  config.plannerVisible = doc["planner_visible"] | true;

  std::vector<std::string> order;
  JsonArray rows = doc["order"].as<JsonArray>();
  if (!rows.isNull()) {
    for (JsonVariant row : rows) {
      const char* value = row.as<const char*>();
      if (!value) continue;
      std::string card = value;
      if (!validCard(card) || hasCard(order, card)) continue;
      order.push_back(std::move(card));
    }
  }

  if (!hasCard(order, "reader")) {
    order.insert(order.begin(), "reader");
  }
  for (const char* card : {"weather", "markets", "planner"}) {
    if (!hasCard(order, card)) order.emplace_back(card);
  }
  config.order = std::move(order);
  return true;
}

bool save(const DashboardConfig& config) {
  if (!ensureRoot()) return false;

  JsonDocument doc;
  doc["reader_visible"] = config.readerVisible;
  doc["weather_visible"] = config.weatherVisible;
  doc["markets_visible"] = config.marketsVisible;
  doc["planner_visible"] = config.plannerVisible;

  JsonArray order = doc["order"].to<JsonArray>();
  for (const auto& card : config.order) {
    if (validCard(card)) order.add(card);
  }

  String raw;
  serializeJsonPretty(doc, raw);
  return Storage.writeFile(CONFIG_PATH, raw);
}

}  // namespace DashboardStore
