#include "PowerSettingsActivity.h"

#include <GfxRenderer.h>
#include <WiFi.h>

#include "PowerManager.h"
#include "components/UITheme.h"

namespace fui = freeink::ui;

namespace {
const char* LABELS[PowerSettingsActivity::ROWS] = {
    "Online przy otwarciu",
    "Wi-Fi OFF po odswiezeniu",
    "Wylacz Wi-Fi teraz",
};
}

void PowerSettingsActivity::onEnter() {
  UiListActivity::onEnter();
  reload();
}

void PowerSettingsActivity::reload() {
  PowerStore::load(config);
  rebuildRows();
}

void PowerSettingsActivity::rebuildRows() {
  values[0] = config.autoRefreshOnOpen ? "AUTO" : "RECZNIE";
  values[1] = config.radioOffAfterRefresh ? "WLACZONE" : "WYLACZONE";
  values[2] = WiFi.status() == WL_CONNECTED ? "POLACZONO" : "ROZLACZONO";

  for (int i = 0; i < ROWS; ++i) {
    rows[i] = {};
    rows[i].label = LABELS[i];
    rows[i].value = values[i].c_str();
    rows[i].actionValue = static_cast<int16_t>(i);
  }
}

void PowerSettingsActivity::toggleAutoRefresh() {
  config.autoRefreshOnOpen = !config.autoRefreshOnOpen;
  header = PowerStore::save(config) ? "Power Manager" : "Blad zapisu Power";
  rebuildRows();
  requestUpdate();
}

void PowerSettingsActivity::toggleRadioOffAfterRefresh() {
  config.radioOffAfterRefresh = !config.radioOffAfterRefresh;
  header = PowerStore::save(config) ? "Power Manager" : "Blad zapisu Power";
  rebuildRows();
  requestUpdate();
}

void PowerSettingsActivity::radioOffNow() {
  header = PowerManager::turnWifiOffNow()
               ? "Wi-Fi OFF"
               : "Nie udalo sie wylaczyc Wi-Fi";
  rebuildRows();
  requestUpdate();
}

void PowerSettingsActivity::activateIndex(const int index) {
  switch (index) {
    case 0: toggleAutoRefresh(); return;
    case 1: toggleRadioOffAfterRefresh(); return;
    case 2: radioOffNow(); return;
    default: return;
  }
}

void PowerSettingsActivity::buildScreen(UiScreen& screen) {
  const auto& metrics = UITheme::getInstance().getMetrics();
  screen.setContentMarginFromScreen(
      fui::Insets{static_cast<int16_t>(metrics.topPadding + metrics.headerHeight), 0,
                  static_cast<int16_t>(metrics.buttonHintsHeight), 0});
  screen.spacer(static_cast<int16_t>(metrics.verticalSpacing));

  fui::ListProps props;
  props.items = rows.data();
  props.count = ROWS;
  props.action = ACTION_ROW;
  props.inputMask = fui::InputTouch;
  props.valueInset = 8;
  props.labelText = screen.theme().smallText;
  props.labelText.maxLines = 2;
  syncListViewport(screen, props);
  screen.list(props);
}
