#pragma once

struct PowerConfig {
  bool autoRefreshOnOpen = false;
  bool radioOffAfterRefresh = false;
};

namespace PowerStore {

constexpr const char* ROOT = "/.x4-multihub/power";
constexpr const char* CONFIG_PATH = "/.x4-multihub/power/config.json";

PowerConfig defaults();
bool load(PowerConfig& config);
bool save(const PowerConfig& config);

}  // namespace PowerStore
