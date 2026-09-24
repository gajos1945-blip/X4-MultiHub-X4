#include "MultiHubActivity.h"

#include <GfxRenderer.h>
#include <memory>

#include "components/UITheme.h"
#include "activities/ActivityManager.h"
#include "FieldManualActivity.h"
#include "DailyPlannerActivity.h"
#include "MarketsActivity.h"
#include "WeatherActivity.h"
#include "NewsActivity.h"
#include "DashboardActivity.h"
#include "MultiHubSettingsActivity.h"

namespace fui = freeink::ui;

namespace {
const char* LABELS[MultiHubActivity::ROWS] = {
    "Czytnik",
    "Instrukcje",
    "Planer",
    "Rynki",
    "Pogoda",
    "Wiadomosci",
    "Dashboard",
    "Ustawienia",
};
}

void MultiHubActivity::onEnter() {
  UiListActivity::onEnter();
  rebuildRows();
}

void MultiHubActivity::rebuildRows() {
  values[0] = "Biblioteka i pliki";
  values[1] = "Manuale offline / checklisty";
  values[2] = "Zadania / priorytety / notatki";
  values[3] = "GPW / NewConnect / Crypto / FX";
  values[4] = "Open-Meteo przez Gateway";
  values[5] = "RSS / Atom / Ulubione";
  values[6] = "Pogoda / Rynki / Planner";
  values[7] = "Gateway / Pogoda / Cache / Dashboard";

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
      activityManager.goToFileBrowser();
      return;
    case 1:
      startActivityForResult(
          std::make_unique<FieldManualActivity>(renderer, mappedInput),
          [this](const ActivityResult&) {
            rebuildRows();
            requestUpdate();
          });
      return;
    case 2:
      startActivityForResult(
          std::make_unique<DailyPlannerActivity>(renderer, mappedInput),
          [this](const ActivityResult&) {
            rebuildRows();
            requestUpdate();
          });
      return;
    case 3:
      startActivityForResult(
          std::make_unique<MarketsActivity>(renderer, mappedInput),
          [this](const ActivityResult&) {
            rebuildRows();
            requestUpdate();
          });
      return;
    case 4:
      startActivityForResult(
          std::make_unique<WeatherActivity>(renderer, mappedInput),
          [this](const ActivityResult&) {
            rebuildRows();
            requestUpdate();
          });
      return;
    case 5:
      startActivityForResult(
          std::make_unique<NewsActivity>(renderer, mappedInput),
          [this](const ActivityResult&) {
            rebuildRows();
            requestUpdate();
          });
      return;
    case 6:
      startActivityForResult(
          std::make_unique<DashboardActivity>(renderer, mappedInput),
          [this](const ActivityResult&) {
            rebuildRows();
            requestUpdate();
          });
      return;
    case 7:
      startActivityForResult(
          std::make_unique<MultiHubSettingsActivity>(renderer, mappedInput),
          [this](const ActivityResult&) {
            rebuildRows();
            requestUpdate();
          });
      return;
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
