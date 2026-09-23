#pragma once

#include <cstdint>
#include <string>
#include <vector>

struct MarketSearchItem {
  std::string symbol;
  std::string name;
  std::string asset;
  std::string currency;
  std::string instrumentType;
};

struct MarketQuote {
  std::string symbol;
  bool available = false;
  double price = 0.0;
  double changePercent = 0.0;
  int64_t timestamp = 0;
  std::string provider;
};

struct MarketSearchResponse {
  bool ok = false;
  std::string error;
  std::vector<MarketSearchItem> items;
};

struct MarketQuoteResponse {
  bool ok = false;
  std::string error;
  std::vector<MarketQuote> items;
};

class MarketGatewayClient {
 public:
  MarketSearchResponse search(const std::string& gateway,
                              const std::string& asset,
                              const std::string& query) const;

  MarketQuoteResponse quotes(const std::string& gateway,
                             const std::vector<std::string>& symbols) const;

 private:
  static std::string encode(const std::string& value);
  static std::string baseUrl(const std::string& gateway);
};
