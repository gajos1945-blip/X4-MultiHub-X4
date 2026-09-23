#include "DataCache.h"

#include <ArduinoJson.h>
#include <HalStorage.h>

#include <algorithm>
#include <string>

namespace {
constexpr const char* MOD = "DataCache";
constexpr size_t MAX_CACHE_BYTES = 24 * 1024;
constexpr size_t MAX_QUOTES = 40;
constexpr size_t MAX_FORECAST = 4;

bool ensureRoot() {
  if (!Storage.ensureDirectoryExists("/.x4-multihub")) return false;
  return Storage.ensureDirectoryExists(DataCache::ROOT);
}

bool readWhole(const char* path, std::string& raw) {
  raw.clear();

  HalFile file;
  if (!Storage.openFileForRead(MOD, path, file)) return false;

  raw.reserve(2048);
  while (file.available() && raw.size() < MAX_CACHE_BYTES) {
    const int ch = file.read();
    if (ch < 0) break;
    raw.push_back(static_cast<char>(ch));
  }

  const bool tooLarge = file.available();
  file.close();
  if (tooLarge) {
    raw.clear();
    return false;
  }
  return !raw.empty();
}

bool writeAtomic(const char* path, const String& raw) {
  if (!ensureRoot() || raw.length() > MAX_CACHE_BYTES) return false;

  const std::string temp = std::string(path) + ".tmp";
  if (!Storage.writeFile(temp.c_str(), raw)) return false;

  Storage.remove(path);
  if (!Storage.rename(temp.c_str(), path)) {
    Storage.remove(temp.c_str());
    return false;
  }
  return true;
}
}  // namespace

namespace DataCache {

bool saveMarketQuotes(const std::vector<MarketQuote>& quotes) {
  JsonDocument doc;
  JsonArray rows = doc["quotes"].to<JsonArray>();

  const size_t count = std::min(quotes.size(), MAX_QUOTES);
  for (size_t i = 0; i < count; ++i) {
    const auto& quote = quotes[i];
    JsonObject row = rows.add<JsonObject>();
    row["symbol"] = quote.symbol;
    row["available"] = quote.available;
    row["price"] = quote.price;
    row["change_p"] = quote.changePercent;
    row["timestamp"] = quote.timestamp;
    row["provider"] = quote.provider;
  }

  String raw;
  serializeJson(doc, raw);
  return writeAtomic(MARKET_QUOTES_PATH, raw);
}

bool loadMarketQuotes(std::vector<MarketQuote>& quotes) {
  quotes.clear();

  std::string raw;
  if (!readWhole(MARKET_QUOTES_PATH, raw)) return false;

  JsonDocument doc;
  if (deserializeJson(doc, raw)) return false;

  JsonArray rows = doc["quotes"].as<JsonArray>();
  if (rows.isNull()) return false;

  for (JsonObject row : rows) {
    if (quotes.size() >= MAX_QUOTES) break;
    const char* symbol = row["symbol"] | nullptr;
    if (!symbol || !*symbol) continue;

    MarketQuote quote;
    quote.symbol = symbol;
    quote.available = row["available"] | false;
    quote.price = row["price"] | 0.0;
    quote.changePercent = row["change_p"] | 0.0;
    quote.timestamp = row["timestamp"] | 0LL;
    quote.provider = row["provider"] | "";
    quotes.push_back(std::move(quote));
  }

  return !quotes.empty();
}

bool saveWeather(const WeatherSnapshot& weather) {
  if (!weather.available) return false;

  JsonDocument doc;
  doc["available"] = weather.available;
  doc["city"] = weather.city;
  doc["country"] = weather.country;
  doc["observed_at"] = weather.observedAt;
  doc["provider"] = weather.provider;
  doc["weather_code"] = weather.weatherCode;

  doc["has_temperature"] = weather.hasTemperature;
  doc["temperature_c"] = weather.temperatureC;
  doc["has_apparent"] = weather.hasApparent;
  doc["apparent_c"] = weather.apparentC;
  doc["has_humidity"] = weather.hasHumidity;
  doc["humidity_percent"] = weather.humidityPercent;
  doc["has_precipitation"] = weather.hasPrecipitation;
  doc["precipitation_mm"] = weather.precipitationMm;
  doc["has_wind"] = weather.hasWind;
  doc["wind_kmh"] = weather.windKmh;

  JsonArray forecast = doc["forecast"].to<JsonArray>();
  const size_t count = std::min(weather.forecast.size(), MAX_FORECAST);
  for (size_t i = 0; i < count; ++i) {
    const auto& day = weather.forecast[i];
    JsonObject row = forecast.add<JsonObject>();
    row["date"] = day.date;
    row["weather_code"] = day.weatherCode;
    row["has_min"] = day.hasMin;
    row["min_c"] = day.minC;
    row["has_max"] = day.hasMax;
    row["max_c"] = day.maxC;
    row["has_pop"] = day.hasPrecipitationProbability;
    row["pop"] = day.precipitationProbability;
  }

  String raw;
  serializeJson(doc, raw);
  return writeAtomic(WEATHER_PATH, raw);
}

bool loadWeather(WeatherSnapshot& weather) {
  weather = {};

  std::string raw;
  if (!readWhole(WEATHER_PATH, raw)) return false;

  JsonDocument doc;
  if (deserializeJson(doc, raw)) return false;

  weather.available = doc["available"] | false;
  weather.city = doc["city"] | "";
  weather.country = doc["country"] | "";
  weather.observedAt = doc["observed_at"] | "";
  weather.provider = doc["provider"] | "";
  weather.weatherCode = doc["weather_code"] | -1;

  weather.hasTemperature = doc["has_temperature"] | false;
  weather.temperatureC = doc["temperature_c"] | 0.0;
  weather.hasApparent = doc["has_apparent"] | false;
  weather.apparentC = doc["apparent_c"] | 0.0;
  weather.hasHumidity = doc["has_humidity"] | false;
  weather.humidityPercent = doc["humidity_percent"] | 0.0;
  weather.hasPrecipitation = doc["has_precipitation"] | false;
  weather.precipitationMm = doc["precipitation_mm"] | 0.0;
  weather.hasWind = doc["has_wind"] | false;
  weather.windKmh = doc["wind_kmh"] | 0.0;

  JsonArray forecast = doc["forecast"].as<JsonArray>();
  if (!forecast.isNull()) {
    for (JsonObject row : forecast) {
      if (weather.forecast.size() >= MAX_FORECAST) break;

      WeatherForecastDay day;
      day.date = row["date"] | "";
      day.weatherCode = row["weather_code"] | -1;
      day.hasMin = row["has_min"] | false;
      day.minC = row["min_c"] | 0.0;
      day.hasMax = row["has_max"] | false;
      day.maxC = row["max_c"] | 0.0;
      day.hasPrecipitationProbability = row["has_pop"] | false;
      day.precipitationProbability = row["pop"] | 0.0;
      weather.forecast.push_back(std::move(day));
    }
  }

  return weather.available;
}

bool clearMarkets() {
  return !Storage.exists(MARKET_QUOTES_PATH) || Storage.remove(MARKET_QUOTES_PATH);
}

bool clearWeather() {
  return !Storage.exists(WEATHER_PATH) || Storage.remove(WEATHER_PATH);
}

}  // namespace DataCache
