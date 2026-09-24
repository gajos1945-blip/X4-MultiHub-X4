#include "ReaderDashboardActivity.h"

#include <GfxRenderer.h>

#include <utility>

#include "activities/ActivityManager.h"
#include "components/UITheme.h"

namespace fui = freeink::ui;

void ReaderDashboardActivity::onEnter() {
  UiListActivity::onEnter();
  reload();
}

void ReaderDashboardActivity::onExit() {
  Activity::onExit();
  rows.clear();
  labels.clear();
  values.clear();
  subtitles.clear();
  actions.clear();
}

void ReaderDashboardActivity::reload() {
  ReaderDashboardStore::load(snapshot);
  rebuildRows();
}

void ReaderDashboardActivity::addRow(
    const std::string& action,
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

void ReaderDashboardActivity::rebuildRows() {
  rows.clear();
  labels.clear();
  values.clear();
  subtitles.clear();
  actions.clear();

  rows.reserve(5);
  labels.reserve(5);
  values.reserve(5);
  subtitles.reserve(5);
  actions.reserve(5);

  if (!snapshot.available) {
    header = "Reader Dashboard";
    addRow("library", "Brak ostatniej ksiazki", "Biblioteka i pliki",
           "Otworz Czytnik i wybierz plik");
    return;
  }

  header = snapshot.title.empty() ? "Reader Dashboard" : snapshot.title;

  addRow("continue",
         "Kontynuuj czytanie",
         ReaderDashboardStore::bookProgressLabel(snapshot),
         snapshot.author.empty() ? snapshot.path : snapshot.author);

  addRow("none",
         "Postep",
         ReaderDashboardStore::pageLabel(snapshot),
         snapshot.bookProgressAvailable
             ? "Potwierdzony z cache CrossPoint"
             : "Postep calej ksiazki UNKNOWN");

  addRow("none", "Plik", snapshot.path, "CrossPoint APP_STATE");
  addRow("library", "Biblioteka i pliki", "Otworz", "Czytnik CrossPoint");
}

void ReaderDashboardActivity::continueReading() {
  if (!snapshot.available || snapshot.path.empty()) {
    openLibrary();
    return;
  }

  app.clearTapFlash();
  activityManager.goToReader(snapshot.path);
}

void ReaderDashboardActivity::openLibrary() {
  app.clearTapFlash();
  activityManager.goToFileBrowser();
}

void ReaderDashboardActivity::activateIndex(const int index) {
  if (index < 0 || index >= static_cast<int>(actions.size())) return;

  const auto action = actions[index];
  if (action == "continue") {
    continueReading();
  } else if (action == "library") {
    openLibrary();
  }
}

void ReaderDashboardActivity::buildScreen(UiScreen& screen) {
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
  props.labelText.maxLines = 3;
  syncListViewport(screen, props);
  screen.list(props);
}
