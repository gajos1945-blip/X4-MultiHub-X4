#include "MultiHubSettingsActivity.h"

#include <GfxRenderer.h>
#include <WiFi.h>

#include <memory>

#include "DataCache.h"
#include "DashboardSettingsActivity.h"
#include "MarketStore.h"
#include "PlannerStore.h"
#include "PowerSettingsActivity.h"
#include "WeatherStore.h"
#include "activities/util/KeyboardEntryActivity.h"
#include "components/UITheme.h"

namespace fui = freeink::ui;

namespace {
const char* LABELS[MultiHubSettingsActivity::ROWS] = {
    "Wi-Fi",
    "X4 Data Gateway",
    "Miasto pogody",
    "Data planera",
    "Uklad Dashboard",
    "Power Manager",
    "Wyczysc cache rynkow",
    "Wyczysc cache pogody",
    "Wyczysc caly cache",
    "O programie",
};
}

void MultiHubSettingsActivity::onEnter() {
  UiListActivity::onEnter();
  reload();
}

void MultiHubSettingsActivity::reload() {
  MarketStore::loadGateway(gateway);
  WeatherStore::loadCity(city);
  PlannerStore::loadActiveDate(plannerDate);
  DashboardStore::load(dashboardConfig);
  rebuildRows();
}

void MultiHubSettingsActivity::rebuildRows() {
  values[0] = WiFi.status() == WL_CONNECTED ? "POLACZONO" : "ROZLACZONO";
  values[1] = gateway.empty() ? "USTAW http://IP:8788" : gateway;
  values[2] = city.empty() ? "USTAW" : city;
  values[3] = PlannerStore::validDate(plannerDate) ? plannerDate : "USTAW YYYY-MM-DD";
  values[4] = "Pogoda / Rynki / Planner";
  values[5] = "Siec / odswiezanie / Wi-Fi OFF";
  values[6] = "market_quotes.json";
  values[7] = "weather.json";
  values[8] = "Rynki + Pogoda";
  values[9] = "X4 MultiHub 1.2-dev";

  for (int i = 0; i < ROWS; ++i) {
    rows[i] = {};
    rows[i].label = LABELS[i];
    rows[i].value = values[i].c_str();
    rows[i].actionValue = static_cast<int16_t>(i);
  }
}

void MultiHubSettingsActivity::editGateway() {
  startActivityForResult(
      std::make_unique<KeyboardEntryActivity>(
          renderer, mappedInput, "Gateway http://IP:8788",
          gateway, 160, InputType::Text),
      [this](const ActivityResult& result) {
        if (!result.isCancelled) {
          const std::string value = std::get<KeyboardResult>(result.data).text;
          if (MarketStore::saveGateway(value)) {
            gateway = value;
            header = "Ustawienia MultiHub";
          } else {
            header = "Niepoprawny gateway";
          }
        }
        rebuildRows();
        requestUpdate();
      });
}

void MultiHubSettingsActivity::editCity() {
  startActivityForResult(
      std::make_unique<KeyboardEntryActivity>(
          renderer, mappedInput, "Miasto, np. Katowice, PL",
          city, 96, InputType::Text),
      [this](const ActivityResult& result) {
        if (!result.isCancelled) {
          const std::string value = std::get<KeyboardResult>(result.data).text;
          if (WeatherStore::saveCity(value)) {
            city = value;
            header = "Ustawienia MultiHub";
          } else {
            header = "Niepoprawne miasto";
          }
        }
        rebuildRows();
        requestUpdate();
      });
}

void MultiHubSettingsActivity::editPlannerDate() {
  startActivityForResult(
      std::make_unique<KeyboardEntryActivity>(
          renderer, mappedInput, "Data YYYY-MM-DD",
          plannerDate, 10, InputType::Text),
      [this](const ActivityResult& result) {
        if (!result.isCancelled) {
          const std::string value = std::get<KeyboardResult>(result.data).text;
          if (PlannerStore::validDate(value) &&
              PlannerStore::saveActiveDate(value)) {
            plannerDate = value;
            header = "Ustawienia MultiHub";
          } else {
            header = "Niepoprawna data";
          }
        }
        rebuildRows();
        requestUpdate();
      });
}

void MultiHubSettingsActivity::openDashboardLayout() {
  startActivityForResult(
      std::make_unique<DashboardSettingsActivity>(
          renderer, mappedInput, dashboardConfig),
      [this](const ActivityResult&) {
        DashboardStore::load(dashboardConfig);
        rebuildRows();
        requestUpdate();
      });
}

void MultiHubSettingsActivity::openPowerManager() {
  startActivityForResult(
      std::make_unique<PowerSettingsActivity>(renderer, mappedInput),
      [this](const ActivityResult&) {
        rebuildRows();
        requestUpdate();
      });
}

void MultiHubSettingsActivity::clearMarketCache() {
  header = DataCache::clearMarkets()
               ? "Cache rynkow wyczyszczony"
               : "Blad czyszczenia cache rynkow";
  requestUpdate();
}

void MultiHubSettingsActivity::clearWeatherCache() {
  header = DataCache::clearWeather()
               ? "Cache pogody wyczyszczony"
               : "Blad czyszczenia cache pogody";
  requestUpdate();
}

void MultiHubSettingsActivity::clearAllCache() {
  const bool markets = DataCache::clearMarkets();
  const bool weather = DataCache::clearWeather();
  header = (markets && weather)
               ? "Caly cache wyczyszczony"
               : "Blad czyszczenia cache";
  requestUpdate();
}

void MultiHubSettingsActivity::activateIndex(const int index) {
  switch (index) {
    case 0:
      header = WiFi.status() == WL_CONNECTED
                   ? "Wi-Fi: POLACZONO"
                   : "Wi-Fi: ROZLACZONO";
      requestUpdate();
      return;
    case 1: editGateway(); return;
    case 2: editCity(); return;
    case 3: editPlannerDate(); return;
    case 4: openDashboardLayout(); return;
    case 5: openPowerManager(); return;
    case 6: clearMarketCache(); return;
    case 7: clearWeatherCache(); return;
    case 8: clearAllCache(); return;
    case 9:
      header = "X4 MultiHub 1.2-dev";
      requestUpdate();
      return;
    default:
      return;
  }
}

void MultiHubSettingsActivity::buildScreen(UiScreen& screen) {
  const auto& metrics = UITheme::getInstance().getMetrics();
  screen.setContentMarginFromScreen(
      fui::Insets{static_cast<int16_t>(metrics.topPadding + metrics.headerHeight), 0,
                  static_cast<int16_t>(metrics.buttonHintsHeight), 0});
  screen.spacer(static_cast<int16_t>(metrics.verticalSpacing));

  fui::ListProps props;
  props.items = rows.data();
  props.count = static_cast<uint16_t>(rows.size());
  props.action = ACTION_ROW;
  props.inputMask = fui::InputTouch;
  props.valueInset = 8;
  props.labelText = screen.theme().smallText;
  props.labelText.maxLines = 2;
  syncListViewport(screen, props);
  screen.list(props);
}
