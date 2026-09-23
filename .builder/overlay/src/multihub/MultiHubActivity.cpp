#include "MultiHubActivity.h"

#include <GfxRenderer.h>

#include "components/UITheme.h"

namespace fui = freeink::ui;

namespace {
const char* LABELS[MultiHubActivity::ROWS] = {
    "Reader",
    "Field Manual",
    "Daily Planner",
    "Markets & Weather",
    "Dashboard",
    "Ustawienia",
};
}

void MultiHubActivity::onEnter() {
  UiListActivity::onEnter();
  rebuildRows();
}

void MultiHubActivity::rebuildRows() {
  values[0] = "CrossPoint Reader - baza gotowa";
  values[1] = "NOT IMPLEMENTED v0.1";
  values[2] = "NOT IMPLEMENTED v0.1";
  values[3] = "NOT IMPLEMENTED v0.1";
  values[4] = "NOT IMPLEMENTED v0.1";
  values[5] = "NOT IMPLEMENTED v0.1";

  for (int i = 0; i < ROWS; ++i) {
    rows[i] = {};
    rows[i].label = LABELS[i];
    rows[i].value = values[i].c_str();
    rows[i].actionValue = static_cast<int16_t>(i);
  }
}

void MultiHubActivity::activateIndex(const int index) {
  switch (index) {
    case 0:
      header = "Reader: uzyj Browse Files na Home (v0.1)";
      break;
    case 1:
      header = "Field Manual - NOT IMPLEMENTED v0.1";
      break;
    case 2:
      header = "Daily Planner - NOT IMPLEMENTED v0.1";
      break;
    case 3:
      header = "Markets & Weather - NOT IMPLEMENTED v0.1";
      break;
    case 4:
      header = "Dashboard - NOT IMPLEMENTED v0.1";
      break;
    case 5:
      header = "Ustawienia - NOT IMPLEMENTED v0.1";
      break;
    default:
      return;
  }
  requestUpdate();
}

void MultiHubActivity::buildScreen(UiScreen& screen) {
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
