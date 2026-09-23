#pragma once

#include <string>

namespace WeatherStore {

constexpr const char* ROOT = "/.x4-multihub/weather";
constexpr const char* CONFIG_PATH = "/.x4-multihub/weather/config.json";

bool loadCity(std::string& city);
bool saveCity(const std::string& city);
bool validCity(const std::string& city);

}  // namespace WeatherStore
