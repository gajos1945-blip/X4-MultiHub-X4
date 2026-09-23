#pragma once

#include <cstddef>
#include <string>
#include <vector>

struct MarketFavorite {
  std::string symbol;
  std::string name;
  std::string asset;     // pl / crypto / fx
  std::string currency;
};

namespace MarketStore {

constexpr size_t MAX_FAVORITES = 40;
constexpr const char* ROOT = "/.x4-multihub/markets";
constexpr const char* FAVORITES_PATH = "/.x4-multihub/markets/favorites.jsonl";
constexpr const char* CONFIG_PATH = "/.x4-multihub/markets/config.json";

bool loadFavorites(std::vector<MarketFavorite>& items);
bool saveFavorites(const std::vector<MarketFavorite>& items);
bool addFavorite(const MarketFavorite& item);
bool removeFavorite(const std::string& symbol);

bool loadGateway(std::string& gateway);
bool saveGateway(const std::string& gateway);
bool validGateway(const std::string& gateway);

}  // namespace MarketStore
