#include "NewsGatewayClient.h"

#include <utility>

#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <WiFi.h>

#include <cctype>
#include <cstdio>

namespace {
constexpr int CONNECT_TIMEOUT_MS = 7000;
constexpr int REQUEST_TIMEOUT_MS = 20000;
constexpr size_t MAX_ARTICLES = 15;

bool gatewayAllowed(const std::string& value) {
  return value.rfind("http://", 0) == 0 &&
         value.size() >= 10 &&
         value.size() <= 160 &&
         value.find(' ') == std::string::npos;
}
}

std::string NewsGatewayClient::baseUrl(const std::string& gateway) {
  std::string base = gateway;
  while (!base.empty() && base.back() == '/') base.pop_back();
  return base;
}

std::string NewsGatewayClient::encode(const std::string& value) {
  std::string out;
  char hex[4]{};
  for (const unsigned char c : value) {
    if (std::isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
      out.push_back(static_cast<char>(c));
    } else {
      std::snprintf(hex, sizeof(hex), "%%%02X", c);
      out += hex;
    }
  }
  return out;
}

NewsFeedResponse NewsGatewayClient::fetch(
    const std::string& gateway,
    const std::string& feedUrl,
    int limit) const {
  NewsFeedResponse result;

  if (WiFi.status() != WL_CONNECTED) {
    result.error = "Wi-Fi nie jest polaczone";
    return result;
  }
  if (!gatewayAllowed(gateway)) {
    result.error = "Ustaw poprawny X4 Data Gateway (http://)";
    return result;
  }
  if (!NewsStore::validFeedUrl(feedUrl)) {
    result.error = "Niepoprawny adres RSS/Atom";
    return result;
  }

  if (limit < 1) limit = 1;
  if (limit > static_cast<int>(MAX_ARTICLES)) limit = static_cast<int>(MAX_ARTICLES);

  const std::string url = baseUrl(gateway) +
      "/v1/news?url=" + encode(feedUrl) +
      "&limit=" + std::to_string(limit);

  HTTPClient http;
  http.setConnectTimeout(CONNECT_TIMEOUT_MS);
  http.setTimeout(REQUEST_TIMEOUT_MS);
  if (!http.begin(url.c_str())) {
    result.error = "Nie mozna otworzyc gateway";
    return result;
  }

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

  result.feedTitle = doc["feed"]["title"] | "";

  JsonArray rows = doc["articles"].as<JsonArray>();
  if (rows.isNull()) {
    result.error = "Brak pola articles";
    return result;
  }

  for (JsonObject row : rows) {
    if (result.articles.size() >= MAX_ARTICLES) break;

    const char* title = row["title"] | nullptr;
    const char* link = row["link"] | nullptr;
    if (!title || !*title || !link || !*link) continue;

    NewsArticle article;
    article.title = title;
    article.link = link;
    article.source = row["source"] | result.feedTitle.c_str();
    article.published = row["published"] | "";
    article.summary = row["summary"] | "";
    result.articles.push_back(std::move(article));
  }

  result.ok = true;
  return result;
}
