#pragma once

#include <string>
#include <vector>

#include "DashboardStore.h"
#include "MarketGatewayClient.h"
#include "MarketStore.h"
#include "PlannerStore.h"
#include "WeatherGatewayClient.h"
#include "activities/UiListActivity.h"

class DashboardActivity final : public UiListActivity {
 private:
  static constexpr size_t MAX_MARKET_ROWS = 6;

  DashboardConfig config;
  std::string gateway;
  std::string city;
  std::string lastError;
  std::string header = "Dashboard";

  WeatherSnapshot weather;
  bool weatherCached = false;
  bool marketsCached = false;
  std::vector<MarketFavorite> favorites;
  std::vector<MarketQuote> quotes;
  std::vector<PlannerTask> tasks;
  std::string activeDate;

  std::vector<std::string> labels;
  std::vector<std::string> values;
  std::vector<std::string> subtitles;
  std::vector<std::string> actions;
  std::vector<freeink::ui::ListItem> rows;

  void reload();
  void refreshAll();
  void rebuildRows();
  void openLayout();
  void openCard(const std::string& action);

  bool cardVisible(const std::string& card) const;
  const MarketQuote* quoteFor(const std::string& symbol) const;
  static std::string quoteValue(const MarketQuote* quote);
  void addRow(const std::string& action,
              std::string label,
              std::string value,
              std::string subtitle = {});

 protected:
  int listCount() const override { return static_cast<int>(rows.size()); }
  void buildScreen(UiScreen& screen) override;
  void activateIndex(int index) override;
  const char* headerTitle() const override { return header.c_str(); }

 public:
  DashboardActivity(GfxRenderer& renderer, MappedInputManager& mappedInput)
      : UiListActivity("Dashboard", renderer, mappedInput) {}

  void onEnter() override;
  void onExit() override;
};
