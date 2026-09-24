#include "NewsFavoritesActivity.h"

#include <GfxRenderer.h>
#include <I18n.h>

#include "activities/ActivityManager.h"
#include "components/UITheme.h"

namespace fui = freeink::ui;

void NewsFavoritesActivity::onEnter() {
  UiListActivity::onEnter();
  reload();
}

void NewsFavoritesActivity::onExit() {
  Activity::onExit();
  rows.clear();
  values.clear();
  subtitles.clear();
  items.clear();
}

void NewsFavoritesActivity::reload() {
  items.clear();
  NewsStore::loadFavorites(items);

  rows.clear();
  values.clear();
  subtitles.clear();

  rows.reserve(items.size());
  values.reserve(items.size());
  subtitles.reserve(items.size());

  for (const auto& item : items) {
    values.push_back(item.published.empty() ? "*" : "* | " + item.published);
    subtitles.push_back(item.source);
  }

  for (size_t i = 0; i < items.size(); ++i) {
    fui::ListItem row;
    row.label = items[i].title.c_str();
    row.value = values[i].c_str();
    row.subtitle = subtitles[i].empty() ? nullptr : subtitles[i].c_str();
    row.actionValue = static_cast<int16_t>(i);
    rows.push_back(row);
  }

  header = "Ulubione (";
  header += std::to_string(items.size());
  header += ")";
}

void NewsFavoritesActivity::openArticle(const int index) {
  if (index < 0 || index >= static_cast<int>(items.size())) return;
  if (!NewsStore::writeArticleText(items[index])) {
    header = "Blad zapisu artykulu";
    requestUpdate();
    return;
  }
  app.clearTapFlash();
  activityManager.goToReader(NewsStore::ARTICLE_PATH);
}

void NewsFavoritesActivity::removeAt(const int index) {
  if (index < 0 || index >= static_cast<int>(items.size())) return;

  bool state = true;
  if (!NewsStore::toggleFavorite(items[index], &state)) {
    header = "Blad Ulubionych";
    requestUpdate();
    return;
  }
  reload();
  requestUpdate();
}

void NewsFavoritesActivity::activateIndex(const int index) {
  openArticle(index);
}

void NewsFavoritesActivity::onRowLongPress(const int index) {
  removeAt(index);
}

void NewsFavoritesActivity::buildScreen(UiScreen& screen) {
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
  props.labelText.maxLines = 3;
  syncListViewport(screen, props, true);
  screen.list(props);
}

void NewsFavoritesActivity::drawFooter() {
  const auto labels = mappedInput.mapLabels(
      "Back", "Open | Hold: remove", tr(STR_DIR_UP), tr(STR_DIR_DOWN));
  GUI.drawButtonHints(renderer, labels.btn1, labels.btn2, labels.btn3, labels.btn4);
}
