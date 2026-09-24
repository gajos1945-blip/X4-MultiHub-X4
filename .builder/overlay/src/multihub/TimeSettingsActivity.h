#pragma once

#include <array>
#include <string>

#include "TimeStore.h"
#include "activities/UiListActivity.h"

class TimeSettingsActivity final : public UiListActivity {
 public:
  static constexpr int ROWS = 4;

 private:
  TimeConfig config;
  std::array<std::string, ROWS> values{};
  std::array<freeink::ui::ListItem, ROWS> rows{};
  std::string header = "Czas / NTP";

  void reload();
  void rebuildRows();
  void cycleTimezone();
  void toggleAutoToday();
  void syncNtp();

 protected:
  int listCount() const override { return ROWS; }
  void buildScreen(UiScreen& screen) override;
  void activateIndex(int index) override;
  const char* headerTitle() const override { return header.c_str(); }

 public:
  TimeSettingsActivity(GfxRenderer& renderer, MappedInputManager& mappedInput)
      : UiListActivity("TimeSettings", renderer, mappedInput) {}

  void onEnter() override;
};
