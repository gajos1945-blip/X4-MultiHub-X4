#include "TimeSettingsActivity.h"

#include <GfxRenderer.h>

#include "PowerManager.h"
#include "TimeService.h"
#include "components/UITheme.h"

namespace fui = freeink::ui;

namespace {
const char* LABELS[TimeSettingsActivity::ROWS] = {
    "Aktualny czas",
    "Strefa czasowa",
    "Planner: automatyczne Today",
    "Synchronizuj NTP",
};
}

void TimeSettingsActivity::onEnter() {
  UiListActivity::onEnter();
  reload();
}

void TimeSettingsActivity::reload() {
  TimeStore::load(config);
  TimeService::applyConfiguredTimezone();
  rebuildRows();
}

void TimeSettingsActivity::rebuildRows() {
  std::string now;
  values[0] = TimeService::localNow(now) ? now : "UNKNOWN";
  values[1] = TimeStore::presetName(config.timezonePreset);
  values[2] = config.autoToday ? "WLACZONE" : "WYLACZONE";
  values[3] = "Wi-Fi -> SNTP";

  for (int i = 0; i < ROWS; ++i) {
    rows[i] = {};
    rows[i].label = LABELS[i];
    rows[i].value = values[i].c_str();
    rows[i].actionValue = static_cast<int16_t>(i);
  }
}

void TimeSettingsActivity::cycleTimezone() {
  config.timezonePreset =
      static_cast<uint8_t>((config.timezonePreset + 1) % TimeStore::PRESET_COUNT);

  if (!TimeStore::save(config) || !TimeService::applyConfiguredTimezone()) {
    header = "Blad strefy czasowej";
  } else {
    header = "Czas / NTP";
  }
  rebuildRows();
  requestUpdate();
}

void TimeSettingsActivity::toggleAutoToday() {
  config.autoToday = !config.autoToday;
  header = TimeStore::save(config) ? "Czas / NTP" : "Blad zapisu czasu";
  rebuildRows();
  requestUpdate();
}

void TimeSettingsActivity::syncNtp() {
  std::string error;
  if (TimeService::syncNow(8000, &error)) {
    TimeStore::load(config);
    header = "NTP OK";
    if (config.autoToday) {
      TimeService::applyTodayToPlanner();
    }
  } else {
    header = error.empty() ? "NTP ERROR" : error;
  }

  PowerManager::afterOnlineOperation();
  rebuildRows();
  requestUpdate();
}

void TimeSettingsActivity::activateIndex(const int index) {
  switch (index) {
    case 0:
      rebuildRows();
      requestUpdate();
      return;
    case 1: cycleTimezone(); return;
    case 2: toggleAutoToday(); return;
    case 3: syncNtp(); return;
    default: return;
  }
}

void TimeSettingsActivity::buildScreen(UiScreen& screen) {
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
