#pragma once

#include <string>
#include <vector>

#include "WeatherGatewayClient.h"
#include "activities/UiListActivity.h"

class WeatherActivity final : public UiListActivity {
 private:
  std::string gateway;
  std::string city;
  std::string lastError;
  std::string header = "Pogoda";
  WeatherSnapshot weather;
  bool cachedData = false;

  std::vector<std::string> labels;
  std::vector<std::string> values;
  std::vector<std::string> subtitles;
  std::vector<freeink::ui::ListItem> rows;

  void reload();
  void rebuildRows();
  void editCity();
  void editGateway();
  void refreshWeather();

 protected:
  int listCount() const override { return static_cast<int>(rows.size()); }
  void buildScreen(UiScreen& screen) override;
  void activateIndex(int index) override;
  const char* headerTitle() const override { return header.c_str(); }

 public:
  WeatherActivity(GfxRenderer& renderer, MappedInputManager& mappedInput)
      : UiListActivity("Weather", renderer, mappedInput) {}

  void onEnter() override;
  void onExit() override;
};
