#include "DashboardActivity.h"

#include <GfxRenderer.h>

#include <cstdio>
#include <memory>

#include "DailyPlannerActivity.h"
#include "DataCache.h"
#include "DashboardSettingsActivity.h"
#include "MarketsActivity.h"
#include "WeatherActivity.h"
#include "WeatherStore.h"
#include "components/UITheme.h"

namespace fui = freeink::ui;

namespace {
std::string number1(const double value, const char* suffix) {
  char buffer[48]{};
  std::snprintf(buffer, sizeof(buffer), "%.1f%s", value, suffix);
  return std::string(buffer);
}
}  // namespace

void DashboardActivity::onEnter() {
  UiListActivity::onEnter();
  reload();
}

void DashboardActivity::onExit() {
  Activity::onExit();
  rows.clear();
  labels.clear();
  values.clear();
  subtitles.clear();
  actions.clear();
  favorites.clear();
  quotes.clear();
  tasks.clear();
  weather.forecast.clear();
}

bool DashboardActivity::cardVisible(const std::string& card) const {
  if (card == "weather") return config.weatherVisible;
  if (card == "markets") return config.marketsVisible;
  if (card == "planner") return config.plannerVisible;
  return false;
}

const MarketQuote* DashboardActivity::quoteFor(const std::string& symbol) const {
  for (const auto& quote : quotes) {
    if (quote.symbol == symbol) return &quote;
  }
  return nullptr;
}

std::string DashboardActivity::quoteValue(const MarketQuote* quote) {
  if (!quote || !quote->available) return "DATA UNAVAILABLE";
  char buffer[48]{};
  std::snprintf(buffer, sizeof(buffer), "%.6g %+.2f%%",
                quote->price, quote->changePercent);
  return std::string(buffer);
}

void DashboardActivity::addRow(const std::string& action,
                               std::string label,
                               std::string value,
                               std::string subtitle) {
  actions.push_back(action);
  labels.push_back(std::move(label));
  values.push_back(std::move(value));
  subtitles.push_back(std::move(subtitle));

  const size_t i = labels.size() - 1;
  fui::ListItem row;
  row.label = labels[i].c_str();
  row.value = values[i].c_str();
  row.subtitle = subtitles[i].empty() ? nullptr : subtitles[i].c_str();
  row.actionValue = static_cast<int16_t>(i);
  rows.push_back(row);
}

void DashboardActivity::reload() {
  DashboardStore::load(config);
  MarketStore::loadGateway(gateway);
  WeatherStore::loadCity(city);
  MarketStore::loadFavorites(favorites);
  PlannerStore::load(tasks);
  PlannerStore::loadActiveDate(activeDate);

  quotes.clear();
  marketsCached = DataCache::loadMarketQuotes(quotes);
  weatherCached = DataCache::loadWeather(weather);

  rebuildRows();
}

void DashboardActivity::refreshAll() {
  lastError.clear();

  if (!city.empty()) {
    WeatherGatewayClient weatherClient;
    const WeatherResponse response = weatherClient.current(gateway, city);
    if (response.ok) {
      weather = response.weather;
      weatherCached = false;
      DataCache::saveWeather(weather);
    } else {
      WeatherSnapshot cached;
      if (DataCache::loadWeather(cached)) {
        weather = std::move(cached);
        weatherCached = true;
        lastError = "Pogoda CACHED: " + response.error;
      } else {
        weather = {};
        weatherCached = false;
        lastError = response.error;
      }
    }
  }

  if (!favorites.empty()) {
    std::vector<std::string> symbols;
    const size_t count = favorites.size() < MAX_MARKET_ROWS
                             ? favorites.size()
                             : MAX_MARKET_ROWS;
    symbols.reserve(count);
    for (size_t i = 0; i < count; ++i) {
      symbols.push_back(favorites[i].symbol);
    }

    MarketGatewayClient marketClient;
    const MarketQuoteResponse response = marketClient.quotes(gateway, symbols);
    if (response.ok) {
      quotes = response.items;
      marketsCached = false;
      DataCache::saveMarketQuotes(quotes);
    } else {
      std::vector<MarketQuote> cached;
      if (DataCache::loadMarketQuotes(cached)) {
        quotes = std::move(cached);
        marketsCached = true;
        if (lastError.empty()) lastError = "Rynki CACHED: " + response.error;
      } else {
        quotes.clear();
        marketsCached = false;
        if (lastError.empty()) lastError = response.error;
      }
    }
  }

  rebuildRows();
  requestUpdate();
}

void DashboardActivity::rebuildRows() {
  rows.clear();
  labels.clear();
  values.clear();
  subtitles.clear();
  actions.clear();

  rows.reserve(16);
  labels.reserve(16);
  values.reserve(16);
  subtitles.reserve(16);
  actions.reserve(16);

  addRow("refresh", "Odswiez Dashboard", "Online");
  addRow("layout", "Uklad Dashboard", "Edytuj");

  for (const auto& card : config.order) {
    if (!cardVisible(card)) continue;

    if (card == "weather") {
      if (weather.available) {
        std::string value = weather.hasTemperature
                                ? number1(weather.temperatureC, " C")
                                : "DATA UNAVAILABLE";
        std::string subtitle = weatherCached ? "CACHED | " : "LIVE | ";
        subtitle += WeatherGatewayClient::codeName(weather.weatherCode);
        if (!weather.city.empty()) {
          subtitle += " | ";
          subtitle += weather.city;
        }
        if (!weather.observedAt.empty()) {
          subtitle += " | ";
          subtitle += weather.observedAt;
        }
        addRow("weather", "Pogoda", value, subtitle);
      } else {
        addRow("weather", "Pogoda", "DATA UNAVAILABLE",
               city.empty() ? "Ustaw miasto" : city);
      }
      continue;
    }

    if (card == "planner") {
      size_t open = 0;
      size_t done = 0;
      if (PlannerStore::validDate(activeDate)) {
        for (const auto& task : tasks) {
          if (task.date != activeDate) continue;
          if (task.done) ++done;
          else ++open;
        }
      }
      std::string value = "Otwarte ";
      value += std::to_string(open);
      value += " | Wykonane ";
      value += std::to_string(done);
      addRow("planner", "Planner", value,
             PlannerStore::validDate(activeDate) ? activeDate : "Ustaw date");
      continue;
    }

    if (card == "markets") {
      addRow("markets", "Rynki", std::to_string(favorites.size()) + " ulubionych");
      const size_t count = favorites.size() < MAX_MARKET_ROWS
                               ? favorites.size()
                               : MAX_MARKET_ROWS;
      for (size_t i = 0; i < count; ++i) {
        std::string subtitle = marketsCached ? "CACHED | " : "LIVE | ";
        subtitle += favorites[i].symbol;
        const MarketQuote* q = quoteFor(favorites[i].symbol);
        if (q && q->timestamp > 0) {
          subtitle += " | t=";
          subtitle += std::to_string(q->timestamp);
        }
        addRow("markets", favorites[i].name,
               quoteValue(q), subtitle);
      }
    }
  }

  header = (weatherCached || marketsCached) ? "Dashboard CACHED" : "Dashboard LIVE";
  if (!lastError.empty()) header += " !";
}

void DashboardActivity::openLayout() {
  startActivityForResult(
      std::make_unique<DashboardSettingsActivity>(
          renderer, mappedInput, config),
      [this](const ActivityResult&) {
        DashboardStore::load(config);
        rebuildRows();
        requestUpdate();
      });
}

void DashboardActivity::openCard(const std::string& action) {
  if (action == "weather") {
    startActivityForResult(
        std::make_unique<WeatherActivity>(renderer, mappedInput),
        [this](const ActivityResult&) {
          reload();
          requestUpdate();
        });
    return;
  }
  if (action == "markets") {
    startActivityForResult(
        std::make_unique<MarketsActivity>(renderer, mappedInput),
        [this](const ActivityResult&) {
          reload();
          requestUpdate();
        });
    return;
  }
  if (action == "planner") {
    startActivityForResult(
        std::make_unique<DailyPlannerActivity>(renderer, mappedInput),
        [this](const ActivityResult&) {
          reload();
          requestUpdate();
        });
  }
}

void DashboardActivity::activateIndex(const int index) {
  if (index < 0 || index >= static_cast<int>(actions.size())) return;

  const std::string action = actions[index];
  if (action == "refresh") {
    refreshAll();
  } else if (action == "layout") {
    openLayout();
  } else {
    openCard(action);
  }
}

void DashboardActivity::buildScreen(UiScreen& screen) {
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
