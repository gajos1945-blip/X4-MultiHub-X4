#include "DashboardSettingsActivity.h"

#include <GfxRenderer.h>
#include <I18n.h>

#include <algorithm>

#include "components/UITheme.h"

namespace fui = freeink::ui;

namespace {
const char* displayName(const std::string& card) {
  if (card == "reader") return "Czytnik";
  if (card == "weather") return "Pogoda";
  if (card == "markets") return "Rynki";
  if (card == "planner") return "Planner";
  return "UNKNOWN";
}
}  // namespace

void DashboardSettingsActivity::onEnter() {
  UiListActivity::onEnter();
  rebuildRows();
}

bool DashboardSettingsActivity::visible(const std::string& card) const {
  if (card == "reader") return config.readerVisible;
  if (card == "weather") return config.weatherVisible;
  if (card == "markets") return config.marketsVisible;
  if (card == "planner") return config.plannerVisible;
  return false;
}

void DashboardSettingsActivity::toggle(const std::string& card) {
  if (card == "reader") config.readerVisible = !config.readerVisible;
  else if (card == "weather") config.weatherVisible = !config.weatherVisible;
  else if (card == "markets") config.marketsVisible = !config.marketsVisible;
  else if (card == "planner") config.plannerVisible = !config.plannerVisible;
}

void DashboardSettingsActivity::moveUp(const size_t index) {
  if (index == 0 || index >= config.order.size()) return;
  std::swap(config.order[index - 1], config.order[index]);
}

void DashboardSettingsActivity::rebuildRows() {
  rows.clear();
  values.clear();
  values.reserve(config.order.size());
  rows.reserve(config.order.size());

  for (const auto& card : config.order) {
    values.push_back(visible(card) ? "Widoczna" : "Ukryta");
  }

  for (size_t i = 0; i < config.order.size(); ++i) {
    fui::ListItem row;
    row.label = displayName(config.order[i]);
    row.value = values[i].c_str();
    row.actionValue = static_cast<int16_t>(i);
    rows.push_back(row);
  }
}

void DashboardSettingsActivity::activateIndex(const int index) {
  if (index < 0 || index >= static_cast<int>(config.order.size())) return;
  toggle(config.order[index]);
  DashboardStore::save(config);
  rebuildRows();
  requestUpdate();
}

void DashboardSettingsActivity::onRowLongPress(const int index) {
  if (index <= 0 || index >= static_cast<int>(config.order.size())) return;
  moveUp(static_cast<size_t>(index));
  DashboardStore::save(config);
  rebuildRows();
  nav.selected = index - 1;
  requestUpdate();
}

void DashboardSettingsActivity::buildScreen(UiScreen& screen) {
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
  props.valueInset = 8;
  props.labelText = screen.theme().smallText;
  props.labelText.maxLines = 2;
  syncListViewport(screen, props, true);
  screen.list(props);
}

void DashboardSettingsActivity::drawFooter() {
  const auto labels = mappedInput.mapLabels(
      "Back", "Show/Hide | Hold: up", tr(STR_DIR_UP), tr(STR_DIR_DOWN));
  GUI.drawButtonHints(renderer, labels.btn1, labels.btn2, labels.btn3, labels.btn4);
}
