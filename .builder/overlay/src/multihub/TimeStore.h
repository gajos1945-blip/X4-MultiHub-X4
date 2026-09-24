#pragma once

#include <cstdint>

struct TimeConfig {
  uint8_t timezonePreset = 0;  // UTC by default; user chooses explicitly.
  bool autoToday = false;     // Conservative default: do not change planner date.
  int64_t lastSyncEpoch = 0;
};

namespace TimeStore {

constexpr const char* ROOT = "/.x4-multihub/time";
constexpr const char* CONFIG_PATH = "/.x4-multihub/time/config.json";
constexpr uint8_t PRESET_COUNT = 4;

TimeConfig defaults();
bool load(TimeConfig& config);
bool save(const TimeConfig& config);

const char* presetName(uint8_t index);
const char* presetRule(uint8_t index);
uint8_t normalizedPreset(uint8_t index);

}  // namespace TimeStore
