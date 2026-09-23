#pragma once

#include <cstdint>
#include <string>
#include <vector>

struct WeatherForecastDay {
  std::string date;
  int weatherCode = -1;
  double minC = 0.0;
  double maxC = 0.0;
  double precipitationProbability = 0.0;
  bool hasMin = false;
  bool hasMax = false;
  bool hasPrecipitationProbability = false;
};

struct WeatherSnapshot {
  bool available = false;
  std::string city;
  std::string country;
  std::string observedAt;
  std::string provider;
  int weatherCode = -1;
  double temperatureC = 0.0;
  double apparentC = 0.0;
  double humidityPercent = 0.0;
  double precipitationMm = 0.0;
  double windKmh = 0.0;
  bool hasTemperature = false;
  bool hasApparent = false;
  bool hasHumidity = false;
  bool hasPrecipitation = false;
  bool hasWind = false;
  std::vector<WeatherForecastDay> forecast;
};

struct WeatherResponse {
  bool ok = false;
  std::string error;
  WeatherSnapshot weather;
};

class WeatherGatewayClient {
 public:
  WeatherResponse current(const std::string& gateway,
                          const std::string& city) const;

  static const char* codeName(int code);

 private:
  static std::string encode(const std::string& value);
  static std::string baseUrl(const std::string& gateway);
};
