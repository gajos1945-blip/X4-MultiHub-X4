#include "MarketStore.h"

#include <ArduinoJson.h>
#include <HalStorage.h>

#include <algorithm>

namespace {
constexpr const char* MOD = "Markets";
constexpr size_t MAX_LINE_BYTES = 2048;

bool ensureRoot() {
  if (!Storage.ensureDirectoryExists("/.x4-multihub")) return false;
  return Storage.ensureDirectoryExists(MarketStore::ROOT);
}

bool readLine(HalFile& file, std::string& line) {
  line.clear();
  while (file.available()) {
    const int ch = file.read();
    if (ch < 0) break;
    if (ch == '\n') return true;
    if (ch == '\r') continue;
    if (line.size() >= MAX_LINE_BYTES) {
      while (file.available()) {
        const int c = file.read();
        if (c < 0 || c == '\n') break;
      }
      line.clear();
      return true;
    }
    line.push_back(static_cast<char>(ch));
  }
  return !line.empty();
}
}  // namespace

namespace MarketStore {

bool validGateway(const std::string& gateway) {
  if (gateway.rfind("http://", 0) != 0) return false;
  if (gateway.size() < 10 || gateway.size() > 160) return false;
  return gateway.find(' ') == std::string::npos;
}

bool loadGateway(std::string& gateway) {
  gateway.clear();
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
  const char* value = doc["gateway"] | nullptr;
  if (!value || !validGateway(value)) return false;
  gateway = value;
  return true;
}

bool saveGateway(const std::string& gateway) {
  if (!validGateway(gateway) || !ensureRoot()) return false;

  JsonDocument doc;
  doc["gateway"] = gateway;
  String raw;
  serializeJsonPretty(doc, raw);
  return Storage.writeFile(CONFIG_PATH, raw);
}

bool loadFavorites(std::vector<MarketFavorite>& items) {
  items.clear();

  HalFile file;
  if (!Storage.openFileForRead(MOD, FAVORITES_PATH, file)) return false;

  std::string line;
  line.reserve(512);
  while (items.size() < MAX_FAVORITES && readLine(file, line)) {
    if (line.empty()) continue;

    JsonDocument doc;
    if (deserializeJson(doc, line)) continue;

    const char* symbol = doc["symbol"] | nullptr;
    const char* name = doc["name"] | nullptr;
    const char* asset = doc["asset"] | nullptr;
    if (!symbol || !*symbol || !name || !*name || !asset || !*asset) continue;

    MarketFavorite item;
    item.symbol = symbol;
    item.name = name;
    item.asset = asset;
    item.currency = doc["currency"] | "";
    items.push_back(std::move(item));
  }
  file.close();
  return true;
}

bool saveFavorites(const std::vector<MarketFavorite>& items) {
  if (!ensureRoot()) return false;

  const std::string temp = std::string(FAVORITES_PATH) + ".tmp";
  HalFile file;
  if (!Storage.openFileForWrite(MOD, temp, file)) return false;

  const size_t count = std::min(items.size(), MAX_FAVORITES);
  for (size_t i = 0; i < count; ++i) {
    JsonDocument doc;
    doc["symbol"] = items[i].symbol;
    doc["name"] = items[i].name;
    doc["asset"] = items[i].asset;
    doc["currency"] = items[i].currency;

    String raw;
    serializeJson(doc, raw);
    if (file.write(reinterpret_cast<const uint8_t*>(raw.c_str()), raw.length()) != raw.length() ||
        file.write(static_cast<uint8_t>('\n')) != 1) {
      file.close();
      Storage.remove(temp.c_str());
      return false;
    }
  }

  file.flush();
  file.close();

  Storage.remove(FAVORITES_PATH);
  if (!Storage.rename(temp.c_str(), FAVORITES_PATH)) {
    Storage.remove(temp.c_str());
    return false;
  }
  return true;
}

bool addFavorite(const MarketFavorite& item) {
  if (item.symbol.empty() || item.name.empty() || item.asset.empty()) return false;

  std::vector<MarketFavorite> items;
  loadFavorites(items);

  const auto it = std::find_if(
      items.begin(), items.end(),
      [&](const MarketFavorite& x) { return x.symbol == item.symbol; });
  if (it != items.end()) return true;
  if (items.size() >= MAX_FAVORITES) return false;

  items.push_back(item);
  return saveFavorites(items);
}

bool removeFavorite(const std::string& symbol) {
  std::vector<MarketFavorite> items;
  if (!loadFavorites(items)) return false;

  const size_t before = items.size();
  items.erase(
      std::remove_if(items.begin(), items.end(),
                     [&](const MarketFavorite& x) { return x.symbol == symbol; }),
      items.end());
  if (items.size() == before) return false;
  return saveFavorites(items);
}

}  // namespace MarketStore
