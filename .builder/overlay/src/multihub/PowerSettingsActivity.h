#pragma once

#include <array>
#include <string>

#include "PowerStore.h"
#include "activities/UiListActivity.h"

class PowerSettingsActivity final : public UiListActivity {
 public:
  static constexpr int ROWS = 3;

 private:
  PowerConfig config;
  std::array<std::string, ROWS> values{};
  std::array<freeink::ui::ListItem, ROWS> rows{};
  std::string header = "Power Manager";

  void reload();
  void rebuildRows();
  void toggleAutoRefresh();
  void toggleRadioOffAfterRefresh();
  void radioOffNow();

 protected:
  int listCount() const override { return ROWS; }
  void buildScreen(UiScreen& screen) override;
  void activateIndex(int index) override;
  const char* headerTitle() const override { return header.c_str(); }

 public:
  PowerSettingsActivity(GfxRenderer& renderer, MappedInputManager& mappedInput)
      : UiListActivity("PowerSettings", renderer, mappedInput) {}

  void onEnter() override;
};
