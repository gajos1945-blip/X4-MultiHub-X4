#pragma once

#include <vector>

#include "MarketGatewayClient.h"
#include "WeatherGatewayClient.h"

namespace DataCache {

constexpr const char* ROOT = "/.x4-multihub/cache";
constexpr const char* MARKET_QUOTES_PATH = "/.x4-multihub/cache/market_quotes.json";
constexpr const char* WEATHER_PATH = "/.x4-multihub/cache/weather.json";

bool saveMarketQuotes(const std::vector<MarketQuote>& quotes);
bool loadMarketQuotes(std::vector<MarketQuote>& quotes);

bool saveWeather(const WeatherSnapshot& weather);
bool loadWeather(WeatherSnapshot& weather);

bool clearMarkets();
bool clearWeather();

}  // namespace DataCache
