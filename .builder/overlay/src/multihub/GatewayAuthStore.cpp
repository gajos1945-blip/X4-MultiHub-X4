#include "GatewayAuthStore.h"

#include <ArduinoJson.h>
#include <HalStorage.h>

#include <cctype>
#include <string>

namespace {
constexpr const char* MOD = "GatewayAuth";

bool ensureRoot() {
  if (!Storage.ensureDirectoryExists("/.x4-multihub")) return false;
  return Storage.ensureDirectoryExists(GatewayAuthStore::ROOT);
}
}

namespace GatewayAuthStore {

bool validToken(const std::string& token) {
  if (token.size() < MIN_TOKEN_LEN || token.size() > MAX_TOKEN_LEN) return false;

  for (const unsigned char c : token) {
    if (std::isspace(c) || c < 0x21 || c > 0x7e) return false;
  }
  return true;
}

bool loadToken(std::string& token) {
  token.clear();

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

  const char* value = doc["token"] | nullptr;
  if (!value || !validToken(value)) return false;

  token = value;
  return true;
}

bool saveToken(const std::string& token) {
  if (!validToken(token) || !ensureRoot()) return false;

  JsonDocument doc;
  doc["token"] = token;

  String raw;
  serializeJson(doc, raw);

  const std::string temp = std::string(CONFIG_PATH) + ".tmp";
  if (!Storage.writeFile(temp.c_str(), raw)) return false;

  Storage.remove(CONFIG_PATH);
  if (!Storage.rename(temp.c_str(), CONFIG_PATH)) {
    Storage.remove(temp.c_str());
    return false;
  }
  return true;
}

bool clearToken() {
  if (!Storage.exists(CONFIG_PATH)) return true;
  return Storage.remove(CONFIG_PATH);
}

bool isConfigured() {
  std::string token;
  return loadToken(token);
}

}  // namespace GatewayAuthStore
