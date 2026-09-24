#pragma once

#include <string>
#include <vector>

struct DashboardConfig {
  std::vector<std::string> order;
  bool readerVisible = true;
  bool weatherVisible = true;
  bool marketsVisible = true;
  bool plannerVisible = true;
};

namespace DashboardStore {

constexpr const char* ROOT = "/.x4-multihub/dashboard";
constexpr const char* CONFIG_PATH = "/.x4-multihub/dashboard/config.json";

DashboardConfig defaults();
bool load(DashboardConfig& config);
bool save(const DashboardConfig& config);
bool validCard(const std::string& card);

}  // namespace DashboardStore
