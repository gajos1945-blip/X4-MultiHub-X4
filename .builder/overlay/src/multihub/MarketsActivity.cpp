#include "MarketsActivity.h"

#include <GfxRenderer.h>
#include <I18n.h>

#include <cstdio>
#include <memory>

#include "MarketSearchResultsActivity.h"
#include "DataCache.h"
#include "activities/util/KeyboardEntryActivity.h"
#include "components/UITheme.h"

namespace fui = freeink::ui;

void MarketsActivity::onEnter() {
  UiListActivity::onEnter();
  reload();
}

void MarketsActivity::onExit() {
  Activity::onExit();
  rows.clear();
  rowLabels.clear();
  rowValues.clear();
  rowSubtitles.clear();
  favorites.clear();
  quotes.clear();
}

const char* MarketsActivity::assetName() const {
  if (asset == "crypto") return "Crypto";
  if (asset == "fx") return "Waluty";
  return "GPW / NewConnect";
}

const MarketQuote* MarketsActivity::quoteFor(const std::string& symbol) const {
  for (const auto& quote : quotes) {
    if (quote.symbol == symbol) return &quote;
  }
  return nullptr;
}

std::string MarketsActivity::priceText(const MarketQuote* quote) {
  if (!quote || !quote->available) return "DATA UNAVAILABLE";

  char buffer[48]{};
  std::snprintf(buffer, sizeof(buffer), "%.6g  %+.2f%%",
                quote->price, quote->changePercent);
  return std::string(buffer);
}

void MarketsActivity::reload() {
  MarketStore::loadGateway(gateway);
  favorites.clear();
  MarketStore::loadFavorites(favorites);

  quotes.clear();
  cachedData = DataCache::loadMarketQuotes(quotes);
  rebuildRows();
}

void MarketsActivity::rebuildRows() {
  rows.clear();
  rowLabels.clear();
  rowValues.clear();
  rowSubtitles.clear();

  rows.reserve(favorites.size() + CONTROL_ROWS);
  rowLabels.reserve(favorites.size());
  rowValues.reserve(favorites.size());
  rowSubtitles.reserve(favorites.size());

  fui::ListItem gatewayRow;
  gatewayRow.label = "X4 Data Gateway";
  gatewayRow.value = gateway.empty() ? "USTAW http://" : gateway.c_str();
  gatewayRow.actionValue = 0;
  rows.push_back(gatewayRow);

  fui::ListItem assetRow;
  assetRow.label = "Rynek wyszukiwania";
  assetRow.value = assetName();
  assetRow.actionValue = 1;
  rows.push_back(assetRow);

  fui::ListItem searchRow;
  searchRow.label = "Szukaj i dodaj";
  searchRow.value = "Nazwa / ticker / ISIN";
  searchRow.actionValue = 2;
  rows.push_back(searchRow);

  fui::ListItem refreshRow;
  refreshRow.label = "Odswiez notowania";
  refreshRow.value = favorites.empty() ? "(brak ulubionych)" : "Gateway";
  refreshRow.actionValue = 3;
  rows.push_back(refreshRow);

  for (const auto& favorite : favorites) {
    rowLabels.push_back(favorite.name);
    rowValues.push_back(priceText(quoteFor(favorite.symbol)));

    std::string subtitle = favorite.symbol;
    if (!favorite.currency.empty()) {
      subtitle += " | ";
      subtitle += favorite.currency;
    }
    if (cachedData) subtitle += " | CACHED";
    const MarketQuote* q = quoteFor(favorite.symbol);
    if (q && q->timestamp > 0) {
      subtitle += " | t=";
      subtitle += std::to_string(q->timestamp);
    }
    rowSubtitles.push_back(std::move(subtitle));
  }

  for (size_t i = 0; i < favorites.size(); ++i) {
    fui::ListItem row;
    row.label = rowLabels[i].c_str();
    row.value = rowValues[i].c_str();
    row.subtitle = rowSubtitles[i].c_str();
    row.actionValue = static_cast<int16_t>(i + CONTROL_ROWS);
    rows.push_back(row);
  }

  header = cachedData ? "Markets CACHED (" : "Markets LIVE (";
  header += std::to_string(favorites.size());
  header += ")";
  if (!lastError.empty()) header += " !";
}

void MarketsActivity::editGateway() {
  startActivityForResult(
      std::make_unique<KeyboardEntryActivity>(
          renderer, mappedInput, "Gateway http://IP:8788", gateway, 160, InputType::Text),
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

void MarketsActivity::cycleAsset() {
  if (asset == "pl") asset = "crypto";
  else if (asset == "crypto") asset = "fx";
  else asset = "pl";
  lastError.clear();
  rebuildRows();
  requestUpdate();
}

void MarketsActivity::searchAndAdd() {
  if (!MarketStore::validGateway(gateway)) {
    lastError = "Najpierw ustaw gateway";
    rebuildRows();
    requestUpdate();
    return;
  }

  startActivityForResult(
      std::make_unique<KeyboardEntryActivity>(
          renderer, mappedInput, "Szukaj: nazwa / ticker / ISIN", "", 80, InputType::Text),
      [this](const ActivityResult& result) {
        if (result.isCancelled) return;

        const std::string query = std::get<KeyboardResult>(result.data).text;
        const MarketSearchResponse response = client.search(gateway, asset, query);
        if (!response.ok) {
          lastError = response.error;
          rebuildRows();
          requestUpdate();
          return;
        }
        if (response.items.empty()) {
          lastError = "DATA UNAVAILABLE";
          rebuildRows();
          requestUpdate();
          return;
        }

        lastError.clear();
        startActivityForResult(
            std::make_unique<MarketSearchResultsActivity>(
                renderer, mappedInput, response.items),
            [this](const ActivityResult&) {
              favorites.clear();
              MarketStore::loadFavorites(favorites);
              rebuildRows();
              requestUpdate();
            });
      });
}

void MarketsActivity::refreshQuotes() {
  lastError.clear();
  quotes.clear();

  if (favorites.empty()) {
    rebuildRows();
    requestUpdate();
    return;
  }

  std::vector<std::string> symbols;
  symbols.reserve(favorites.size());
  for (const auto& favorite : favorites) symbols.push_back(favorite.symbol);

  const MarketQuoteResponse response = client.quotes(gateway, symbols);
  if (!response.ok) {
    std::vector<MarketQuote> cached;
    if (DataCache::loadMarketQuotes(cached)) {
      quotes = std::move(cached);
      cachedData = true;
      lastError = "CACHED: " + response.error;
    } else {
      cachedData = false;
      lastError = response.error;
    }
  } else {
    quotes = response.items;
    cachedData = false;
    lastError.clear();
    DataCache::saveMarketQuotes(quotes);
  }
  rebuildRows();
  requestUpdate();
}

void MarketsActivity::removeFavoriteAt(const int favoriteIndex) {
  if (favoriteIndex < 0 || favoriteIndex >= static_cast<int>(favorites.size())) return;

  const std::string symbol = favorites[favoriteIndex].symbol;
  if (MarketStore::removeFavorite(symbol)) {
    favorites.erase(favorites.begin() + favoriteIndex);
    for (auto it = quotes.begin(); it != quotes.end();) {
      if (it->symbol == symbol) it = quotes.erase(it);
      else ++it;
    }
    lastError.clear();
  } else {
    lastError = "Blad usuwania";
  }
  rebuildRows();
  requestUpdate();
}

void MarketsActivity::activateIndex(const int index) {
  switch (index) {
    case 0: editGateway(); return;
    case 1: cycleAsset(); return;
    case 2: searchAndAdd(); return;
    case 3: refreshQuotes(); return;
    default:
      refreshQuotes();
      return;
  }
}

void MarketsActivity::onRowLongPress(const int index) {
  if (index < CONTROL_ROWS) return;
  removeFavoriteAt(index - CONTROL_ROWS);
}

void MarketsActivity::buildScreen(UiScreen& screen) {
  const auto& metrics = UITheme::getInstance().getMetrics();
  screen.setContentMarginFromScreen(
      fui::Insets{static_cast<int16_t>(metrics.topPadding + metrics.headerHeight), 0,
                  static_cast<int16_t>(metrics.buttonHintsHeight), 0});
  screen.spacer(static_cast<int16_t>(metrics.verticalSpacing));

  fui::ListProps props;
  props.items = rows.data();
  props.count = static_cast<uint16_t>(rows.size());
  props.action = ACTION_ROW;
  props.inputMask = fui::InputTouch | fui::InputLongPress;
  fui::TextStyle label = screen.theme().smallText;
  label.bold = true;
  props.labelText = label;
  props.valueInset = 8;
  syncListViewport(screen, props, true);
  screen.list(props);
}

void MarketsActivity::drawFooter() {
  const auto labels = mappedInput.mapLabels(
      "Back",
      nav.selected >= CONTROL_ROWS ? "Refresh / Hold: remove" : "Select",
      tr(STR_DIR_UP), tr(STR_DIR_DOWN));
  GUI.drawButtonHints(renderer, labels.btn1, labels.btn2, labels.btn3, labels.btn4);
}
