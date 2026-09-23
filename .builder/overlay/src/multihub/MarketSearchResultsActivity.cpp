#include "MarketSearchResultsActivity.h"

#include <GfxRenderer.h>

#include "MarketStore.h"
#include "components/UITheme.h"

namespace fui = freeink::ui;

void MarketSearchResultsActivity::onEnter() {
  UiListActivity::onEnter();
  rebuildRows();
}

void MarketSearchResultsActivity::rebuildRows() {
  rows.clear();
  values.clear();

  values.reserve(items.size());
  rows.reserve(items.size());

  for (const auto& item : items) {
    std::string value = item.symbol;
    if (!item.currency.empty()) {
      value += " | ";
      value += item.currency;
    }
    values.push_back(std::move(value));
  }

  for (size_t i = 0; i < items.size(); ++i) {
    fui::ListItem row;
    row.label = items[i].name.c_str();
    row.value = values[i].c_str();
    row.actionValue = static_cast<int16_t>(i);
    rows.push_back(row);
  }

  header = "Wyniki (";
  header += std::to_string(items.size());
  header += ")";
}

void MarketSearchResultsActivity::activateIndex(const int index) {
  if (index < 0 || index >= static_cast<int>(items.size())) return;

  MarketFavorite favorite;
  favorite.symbol = items[index].symbol;
  favorite.name = items[index].name;
  favorite.asset = items[index].asset;
  favorite.currency = items[index].currency;

  if (MarketStore::addFavorite(favorite)) {
    header = "Dodano: " + favorite.symbol;
  } else {
    header = "Nie mozna dodac";
  }
  requestUpdate();
}

void MarketSearchResultsActivity::buildScreen(UiScreen& screen) {
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
