#include "TimeService.h"

#include <Arduino.h>
#include <WiFi.h>
#include <esp_sntp.h>

#include <cstdio>
#include <cstdlib>
#include <ctime>

#include "PlannerStore.h"
#include "TimeStore.h"

namespace {
bool validEpoch(const int64_t epoch) {
  return epoch >= TimeService::MIN_VALID_EPOCH;
}

void setError(std::string* error, const char* value) {
  if (error) *error = value;
}
}  // namespace

namespace TimeService {

bool applyConfiguredTimezone() {
  TimeConfig config;
  TimeStore::load(config);

  const char* rule = TimeStore::presetRule(config.timezonePreset);
  if (!rule || !*rule) return false;

  if (setenv("TZ", rule, 1) != 0) return false;
  tzset();
  return true;
}

int64_t nowEpoch() {
  const time_t now = time(nullptr);
  const int64_t epoch = static_cast<int64_t>(now);
  return validEpoch(epoch) ? epoch : 0;
}

bool hasValidTime() {
  return nowEpoch() != 0;
}

bool localNow(std::string& value) {
  value.clear();
  applyConfiguredTimezone();

  const time_t now = time(nullptr);
  if (!validEpoch(static_cast<int64_t>(now))) return false;

  struct tm local {};
  if (!localtime_r(&now, &local)) return false;

  char buffer[32]{};
  if (std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M", &local) == 0) {
    return false;
  }
  value = buffer;
  return true;
}

bool today(std::string& yyyyMmDd) {
  yyyyMmDd.clear();
  applyConfiguredTimezone();

  const time_t now = time(nullptr);
  if (!validEpoch(static_cast<int64_t>(now))) return false;

  struct tm local {};
  if (!localtime_r(&now, &local)) return false;

  char buffer[16]{};
  if (std::strftime(buffer, sizeof(buffer), "%Y-%m-%d", &local) == 0) {
    return false;
  }
  yyyyMmDd = buffer;
  return PlannerStore::validDate(yyyyMmDd);
}

bool syncNow(const uint32_t timeoutMs, std::string* error) {
  if (error) error->clear();

  if (WiFi.status() != WL_CONNECTED) {
    setError(error, "Wi-Fi nie jest polaczone");
    return false;
  }

  if (!applyConfiguredTimezone()) {
    setError(error, "Blad strefy czasowej");
    return false;
  }

  // Clear the status before requesting a fresh sync so an already-valid
  // system clock cannot be mistaken for a successful new NTP transaction.
  esp_sntp_set_sync_status(SNTP_SYNC_STATUS_RESET);

  if (esp_sntp_enabled()) {
    // Reuse the upstream CrossPoint/ESP-IDF SNTP service if it already exists.
    // Restart requests a fresh synchronization without replacing its internals.
    esp_sntp_restart();
  } else {
    esp_sntp_setoperatingmode(ESP_SNTP_OPMODE_POLL);
    esp_sntp_setservername(0, "pool.ntp.org");
    esp_sntp_setservername(1, "time.google.com");
    esp_sntp_init();
  }

  const uint32_t started = millis();
  while (millis() - started < timeoutMs) {
    if (esp_sntp_get_sync_status() == SNTP_SYNC_STATUS_COMPLETED) {
      const int64_t epoch = nowEpoch();
      if (validEpoch(epoch)) {
        TimeConfig config;
        TimeStore::load(config);
        config.lastSyncEpoch = epoch;
        TimeStore::save(config);
        return true;
      }
    }
    delay(100);
  }

  setError(error, "NTP timeout");
  return false;
}

bool autoTodayEnabled() {
  TimeConfig config;
  TimeStore::load(config);
  return config.autoToday;
}

bool applyTodayToPlanner(std::string* appliedDate) {
  if (!autoTodayEnabled()) return false;

  std::string date;
  if (!today(date)) return false;

  std::string current;
  PlannerStore::loadActiveDate(current);
  if (current != date && !PlannerStore::saveActiveDate(date)) return false;

  if (appliedDate) *appliedDate = date;
  return true;
}

std::string cacheAgeLabel(const int64_t cachedAtEpoch) {
  const int64_t now = nowEpoch();
  if (!validEpoch(now) || !validEpoch(cachedAtEpoch) || cachedAtEpoch > now + 60) {
    return "CACHED";
  }

  const int64_t age = now - cachedAtEpoch;
  if (age < 60) return "CACHED <1m";

  const int64_t minutes = age / 60;
  if (minutes < 60) {
    return "CACHED " + std::to_string(minutes) + "m";
  }

  const int64_t hours = minutes / 60;
  if (hours < 48) {
    return "CACHED " + std::to_string(hours) + "h";
  }

  const int64_t days = hours / 24;
  return "CACHED " + std::to_string(days) + "d";
}

}  // namespace TimeService
