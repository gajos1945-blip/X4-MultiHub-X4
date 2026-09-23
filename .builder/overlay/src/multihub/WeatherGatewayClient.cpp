#include "WeatherGatewayClient.h"

#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <WiFi.h>

#include <cctype>
#include <cstdio>

namespace {
constexpr int CONNECT_TIMEOUT_MS = 7000;
constexpr int REQUEST_TIMEOUT_MS = 20000;
constexpr size_t MAX_FORECAST_DAYS = 4;

bool gatewayAllowed(const std::string& value) {
  return value.rfind("http://", 0) == 0 &&
         value.size() >= 10 &&
         value.size() <= 160 &&
         value.find(' ') == std::string::npos;
}

bool readNumber(JsonVariantConst value, double& out) {
  if (value.isNull()) return false;
  if (!value.is<float>() && !value.is<double>() &&
      !value.is<int>() && !value.is<long>() &&
      !value.is<unsigned int>() && !value.is<unsigned long>()) {
    return false;
  }
  out = value.as<double>();
  return true;
}
}  // namespace

std::string WeatherGatewayClient::baseUrl(const std::string& gateway) {
  std::string base = gateway;
  while (!base.empty() && base.back() == '/') base.pop_back();
  return base;
}

std::string WeatherGatewayClient::encode(const std::string& value) {
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

const char* WeatherGatewayClient::codeName(const int code) {
  switch (code) {
    case 0: return "Bezchmurnie";
    case 1: return "Prawie bezchmurnie";
    case 2: return "Czesciowe zachmurzenie";
    case 3: return "Pochmurno";
    case 45:
    case 48: return "Mgla";
    case 51:
    case 53:
    case 55:
    case 56:
    case 57: return "Mzawka";
    case 61:
    case 63:
    case 65:
    case 66:
    case 67: return "Deszcz";
    case 71:
    case 73:
    case 75:
    case 77: return "Snieg";
    case 80:
    case 81:
    case 82: return "Przelotny deszcz";
    case 85:
    case 86: return "Przelotny snieg";
    case 95:
    case 96:
    case 99: return "Burza";
    default: return "Kod WMO";
  }
}

WeatherResponse WeatherGatewayClient::current(
    const std::string& gateway,
    const std::string& city) const {
  WeatherResponse result;

  if (WiFi.status() != WL_CONNECTED) {
    result.error = "Wi-Fi nie jest polaczone";
    return result;
  }
  if (!gatewayAllowed(gateway)) {
    result.error = "Ustaw poprawny X4 Data Gateway (http://)";
    return result;
  }
  if (city.empty()) {
    result.error = "Najpierw ustaw miasto";
    return result;
  }

  const std::string url = baseUrl(gateway) +
      "/v1/weather?city=" + encode(city);

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

  JsonObject place = doc["place"].as<JsonObject>();
  if (!place.isNull()) {
    result.weather.city = place["name"] | "";
    result.weather.country = place["country"] | "";
  }
  result.weather.observedAt = doc["observed_at"] | "";
  result.weather.provider = doc["provider"] | "Open-Meteo";

  const int code = doc["weather_code"] | -1;
  result.weather.weatherCode = code;

  result.weather.hasTemperature =
      readNumber(doc["temperature_c"], result.weather.temperatureC);
  result.weather.hasApparent =
      readNumber(doc["apparent_temperature_c"], result.weather.apparentC);
  result.weather.hasHumidity =
      readNumber(doc["humidity_percent"], result.weather.humidityPercent);
  result.weather.hasPrecipitation =
      readNumber(doc["precipitation_mm"], result.weather.precipitationMm);
  result.weather.hasWind =
      readNumber(doc["wind_kmh"], result.weather.windKmh);

  JsonArray forecast = doc["forecast"].as<JsonArray>();
  if (!forecast.isNull()) {
    for (JsonObject row : forecast) {
      if (result.weather.forecast.size() >= MAX_FORECAST_DAYS) break;
      WeatherForecastDay day;
      day.date = row["date"] | "";
      day.weatherCode = row["weather_code"] | -1;
      day.hasMin = readNumber(row["min_c"], day.minC);
      day.hasMax = readNumber(row["max_c"], day.maxC);
      day.hasPrecipitationProbability =
          readNumber(row["precipitation_probability_max"],
                     day.precipitationProbability);
      result.weather.forecast.push_back(std::move(day));
    }
  }

  result.weather.available =
      !result.weather.city.empty() && result.weather.hasTemperature;
  if (!result.weather.available) {
    result.error = "DATA UNAVAILABLE";
    return result;
  }

  result.ok = true;
  return result;
}
