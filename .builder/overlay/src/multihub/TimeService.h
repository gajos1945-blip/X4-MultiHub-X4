#pragma once

#include <cstdint>
#include <string>

namespace TimeService {

constexpr int64_t MIN_VALID_EPOCH = 1704067200LL;  // 2024-01-01 UTC

bool applyConfiguredTimezone();
bool hasValidTime();
int64_t nowEpoch();

bool localNow(std::string& value);
bool today(std::string& yyyyMmDd);

// Start or restart SNTP only when Wi-Fi is already connected.
// Does not connect Wi-Fi itself and does not alter saved Wi-Fi credentials.
bool syncNow(uint32_t timeoutMs, std::string* error = nullptr);

bool autoTodayEnabled();

// If Auto Today is enabled and system time is valid, update the planner's
// active date. Returns true only when a valid Today date was available.
bool applyTodayToPlanner(std::string* appliedDate = nullptr);

// Human-readable age for cache metadata. Returns "CACHED" if system time or
// cache timestamp is unavailable/invalid.
std::string cacheAgeLabel(int64_t cachedAtEpoch);

}  // namespace TimeService
