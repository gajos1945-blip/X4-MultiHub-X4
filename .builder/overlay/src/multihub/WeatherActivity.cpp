#include "WeatherActivity.h"

#include <GfxRenderer.h>

#include <cstdio>
#include <memory>

#include "MarketStore.h"
#include "DataCache.h"
#include "PowerManager.h"
#include "WeatherStore.h"
#include "activities/util/KeyboardEntryActivity.h"
#include "components/UITheme.h"

namespace fui = freeink::ui;

namespace {
std::string number1(const double value, const char* suffix) {
  char buffer[48]{};
  std::snprintf(buffer, sizeof(buffer), "%.1f%s", value, suffix);
  return std::string(buffer);
}

std::string forecastValue(const WeatherForecastDay& day) {
  if (!day.hasMin || !day.hasMax) return "DATA UNAVAILABLE";
  char buffer[48]{};
  std::snprintf(buffer, sizeof(buffer), "%.0f..%.0f C", day.minC, day.maxC);
  return std::string(buffer);
}
}  // namespace

void WeatherActivity::onEnter() {
  UiListActivity::onEnter();
  reload();
}

void WeatherActivity::onExit() {
  Activity::onExit();
  rows.clear();
  labels.clear();
  values.clear();
  subtitles.clear();
  weather.forecast.clear();
}

void WeatherActivity::reload() {
  MarketStore::loadGateway(gateway);
  WeatherStore::loadCity(city);
  cachedData = DataCache::loadWeather(weather);
  rebuildRows();
}

void WeatherActivity::rebuildRows() {
  rows.clear();
  labels.clear();
  values.clear();
  subtitles.clear();

  labels.reserve(16);
  values.reserve(16);
  subtitles.reserve(16);
  rows.reserve(16);

  auto add = [&](std::string label, std::string value, std::string subtitle = {}) {
    labels.push_back(std::move(label));
    values.push_back(std::move(value));
    subtitles.push_back(std::move(subtitle));

    fui::ListItem row;
    const size_t i = labels.size() - 1;
    row.label = labels[i].c_str();
    row.value = values[i].c_str();
    row.subtitle = subtitles[i].empty() ? nullptr : subtitles[i].c_str();
    row.actionValue = static_cast<int16_t>(i);
    rows.push_back(row);
  };

  add("Miasto", city.empty() ? "USTAW" : city);
  add("X4 Data Gateway", gateway.empty() ? "USTAW http://" : gateway);
  add("Odswiez pogode", weather.available ? "Ponownie" : "Online");

  if (weather.available) {
    std::string place = weather.city;
    if (!weather.country.empty()) {
      place += ", ";
      place += weather.country;
    }
    std::string sourceTime = weather.observedAt;
    if (cachedData) {
      sourceTime = "CACHED | " + sourceTime;
    } else {
      sourceTime = "LIVE | " + sourceTime;
    }
    add("Lokalizacja", place, sourceTime);
    add("Warunki", WeatherGatewayClient::codeName(weather.weatherCode),
        "WMO " + std::to_string(weather.weatherCode));
    add("Temperatura",
        weather.hasTemperature ? number1(weather.temperatureC, " C")
                               : "DATA UNAVAILABLE");
    add("Odczuwalna",
        weather.hasApparent ? number1(weather.apparentC, " C")
                            : "DATA UNAVAILABLE");
    add("Wilgotnosc",
        weather.hasHumidity ? number1(weather.humidityPercent, "%")
                            : "DATA UNAVAILABLE");
    add("Opad",
        weather.hasPrecipitation ? number1(weather.precipitationMm, " mm")
                                 : "DATA UNAVAILABLE");
    add("Wiatr",
        weather.hasWind ? number1(weather.windKmh, " km/h")
                        : "DATA UNAVAILABLE");

    for (const auto& day : weather.forecast) {
      std::string subtitle = WeatherGatewayClient::codeName(day.weatherCode);
      if (day.hasPrecipitationProbability) {
        subtitle += " | opad ";
        subtitle += number1(day.precipitationProbability, "%");
      }
      add(day.date.empty() ? "Prognoza" : day.date,
          forecastValue(day), subtitle);
    }
  }

  header = cachedData ? "Pogoda CACHED" : "Pogoda LIVE";
  if (!lastError.empty()) header += " !";
}

void WeatherActivity::editCity() {
  startActivityForResult(
      std::make_unique<KeyboardEntryActivity>(
          renderer, mappedInput, "Miasto, np. Katowice, PL",
          city, 96, InputType::Text),
      [this](const ActivityResult& result) {
        if (!result.isCancelled) {
          const std::string value = std::get<KeyboardResult>(result.data).text;
          if (WeatherStore::saveCity(value)) {
            city = value;
            weather = {};
            cachedData = false;
            lastError.clear();
          } else {
            lastError = "Niepoprawne miasto";
          }
        }
        rebuildRows();
        requestUpdate();
      });
}

void WeatherActivity::editGateway() {
  startActivityForResult(
      std::make_unique<KeyboardEntryActivity>(
          renderer, mappedInput, "Gateway http://IP:8788",
          gateway, 160, InputType::Text),
      [this](const ActivityResult& result) {
        if (!result.isCancelled) {
          const std::string value = std::get<KeyboardResult>(result.data).text;
          if (MarketStore::saveGateway(value)) {
            gateway = value;
            lastError.clear();
          } else {
            lastError = "Niepoprawny gateway";
          }
        }
        rebuildRows();
        requestUpdate();
      });
}

void WeatherActivity::refreshWeather() {
  WeatherGatewayClient client;
  const WeatherResponse response = client.current(gateway, city);
  PowerManager::afterOnlineOperation();
  if (!response.ok) {
    WeatherSnapshot cached;
    if (DataCache::loadWeather(cached)) {
      weather = std::move(cached);
      cachedData = true;
      lastError = "CACHED: " + response.error;
    } else {
      weather = {};
      cachedData = false;
      lastError = response.error;
    }
  } else {
    weather = response.weather;
    cachedData = false;
    lastError.clear();
    DataCache::saveWeather(weather);
  }
  rebuildRows();
  requestUpdate();
}

void WeatherActivity::activateIndex(const int index) {
  if (index == 0) {
    editCity();
    return;
  }
  if (index == 1) {
    editGateway();
    return;
  }
  if (index == 2) {
    refreshWeather();
    return;
  }
}

void WeatherActivity::buildScreen(UiScreen& screen) {
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
