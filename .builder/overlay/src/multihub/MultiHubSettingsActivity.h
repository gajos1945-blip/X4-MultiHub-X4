#pragma once

#include <array>
#include <string>

#include "DashboardStore.h"
#include "activities/UiListActivity.h"

class MultiHubSettingsActivity final : public UiListActivity {
 public:
  static constexpr int ROWS = 12;

 private:

  std::array<std::string, ROWS> values{};
  std::array<freeink::ui::ListItem, ROWS> rows{};
  std::string header = "Ustawienia MultiHub";

  std::string gateway;
  std::string city;
  std::string plannerDate;
  DashboardConfig dashboardConfig;

  void reload();
  void rebuildRows();

  void editGateway();
  void editGatewayToken();
  void editCity();
  void editPlannerDate();
  void openDashboardLayout();
  void openPowerManager();
  void openTimeSettings();
  void clearMarketCache();
  void clearWeatherCache();
  void clearAllCache();

 protected:
  int listCount() const override { return ROWS; }
  void buildScreen(UiScreen& screen) override;
  void activateIndex(int index) override;
  const char* headerTitle() const override { return header.c_str(); }

 public:
  MultiHubSettingsActivity(GfxRenderer& renderer, MappedInputManager& mappedInput)
      : UiListActivity("MultiHubSettings", renderer, mappedInput) {}

  void onEnter() override;
};
