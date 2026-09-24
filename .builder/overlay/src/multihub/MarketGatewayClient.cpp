#include "MarketGatewayClient.h"

#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <WiFi.h>

#include "GatewayAuth.h"

#include <cctype>
#include <cstdio>

namespace {
constexpr int CONNECT_TIMEOUT_MS = 7000;
constexpr int REQUEST_TIMEOUT_MS = 20000;
constexpr size_t MAX_SEARCH_RESULTS = 30;
constexpr size_t MAX_QUOTES = 40;

bool gatewayAllowed(const std::string& value) {
  return value.rfind("http://", 0) == 0 &&
         value.size() >= 10 &&
         value.size() <= 160 &&
         value.find(' ') == std::string::npos;
}
}  // namespace

std::string MarketGatewayClient::baseUrl(const std::string& gateway) {
  std::string base = gateway;
  while (!base.empty() && base.back() == '/') base.pop_back();
  return base;
}

std::string MarketGatewayClient::encode(const std::string& value) {
  std::string out;
  char hex[4]{};
  for (const unsigned char c : value) {
    if (std::isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~' || c == ',') {
      out.push_back(static_cast<char>(c));
    } else {
      std::snprintf(hex, sizeof(hex), "%%%02X", c);
      out += hex;
    }
  }
  return out;
}

MarketSearchResponse MarketGatewayClient::search(
    const std::string& gateway,
    const std::string& asset,
    const std::string& query) const {
  MarketSearchResponse result;

  if (WiFi.status() != WL_CONNECTED) {
    result.error = "Wi-Fi nie jest polaczone";
    return result;
  }
  if (!gatewayAllowed(gateway)) {
    result.error = "Ustaw poprawny X4 Data Gateway (http://)";
    return result;
  }
  if (query.empty()) {
    result.error = "Puste zapytanie";
    return result;
  }
  if (asset != "pl" && asset != "crypto" && asset != "fx") {
    result.error = "Nieznany typ aktywa";
    return result;
  }

  const std::string url = baseUrl(gateway) +
      "/v1/search?asset=" + encode(asset) + "&q=" + encode(query);

  HTTPClient http;
  http.setConnectTimeout(CONNECT_TIMEOUT_MS);
  http.setTimeout(REQUEST_TIMEOUT_MS);
  if (!http.begin(url.c_str())) {
    result.error = "Nie mozna otworzyc gateway";
    return result;
  }

  GatewayAuth::addHeaderIfConfigured(http);
  const int status = http.GET();
  if (status != 200) {
    result.error = "Gateway HTTP " + std::to_string(status);
    http.end();
    return result;
  }

  const String body = http.getString();
  http.end();

  JsonDocument doc;
  if (deserializeJson(doc, body)) {
    result.error = "Nieprawidlowa odpowiedz JSON";
    return result;
  }

  JsonArray rows = doc["results"].as<JsonArray>();
  if (rows.isNull()) {
    result.error = "Brak pola results";
    return result;
  }

  for (JsonObject row : rows) {
    if (result.items.size() >= MAX_SEARCH_RESULTS) break;
    const char* symbol = row["symbol"] | nullptr;
    const char* name = row["name"] | nullptr;
    if (!symbol || !*symbol || !name || !*name) continue;

    MarketSearchItem item;
    item.symbol = symbol;
    item.name = name;
    item.asset = asset;
    item.currency = row["currency"] | "";
    item.instrumentType = row["instrument_type"] | "";
    result.items.push_back(std::move(item));
  }

  result.ok = true;
  return result;
}

MarketQuoteResponse MarketGatewayClient::quotes(
    const std::string& gateway,
    const std::vector<std::string>& symbols) const {
  MarketQuoteResponse result;

  if (WiFi.status() != WL_CONNECTED) {
    result.error = "Wi-Fi nie jest polaczone";
    return result;
  }
  if (!gatewayAllowed(gateway)) {
    result.error = "Ustaw poprawny X4 Data Gateway (http://)";
    return result;
  }
  if (symbols.empty()) {
    result.ok = true;
    return result;
  }
  if (symbols.size() > MAX_QUOTES) {
    result.error = "Za duzo instrumentow";
    return result;
  }

  std::string joined;
  for (size_t i = 0; i < symbols.size(); ++i) {
    if (i) joined += ',';
    joined += symbols[i];
  }

  const std::string url = baseUrl(gateway) +
      "/v1/quotes?symbols=" + encode(joined);

  HTTPClient http;
  http.setConnectTimeout(CONNECT_TIMEOUT_MS);
  http.setTimeout(REQUEST_TIMEOUT_MS);
  if (!http.begin(url.c_str())) {
    result.error = "Nie mozna otworzyc gateway";
    return result;
  }

  GatewayAuth::addHeaderIfConfigured(http);
  const int status = http.GET();
  if (status != 200) {
    result.error = "Gateway HTTP " + std::to_string(status);
    http.end();
    return result;
  }

  const String body = http.getString();
  http.end();

  JsonDocument doc;
  if (deserializeJson(doc, body)) {
    result.error = "Nieprawidlowa odpowiedz JSON";
    return result;
  }

  JsonArray rows = doc["quotes"].as<JsonArray>();
  if (rows.isNull()) {
    result.error = "Brak pola quotes";
    return result;
  }

  for (JsonObject row : rows) {
    const char* symbol = row["symbol"] | nullptr;
    if (!symbol || !*symbol) continue;

    MarketQuote quote;
    quote.symbol = symbol;
    quote.available = row["available"] | false;
    quote.price = row["price"] | 0.0;
    quote.changePercent = row["change_p"] | 0.0;
    quote.timestamp = row["timestamp"] | 0LL;
    quote.provider = row["provider"] | "";
    result.items.push_back(std::move(quote));
  }

  result.ok = true;
  return result;
}
