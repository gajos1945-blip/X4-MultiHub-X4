#include "NewsFeedActivity.h"

#include <GfxRenderer.h>
#include <I18n.h>

#include "MarketStore.h"
#include "activities/ActivityManager.h"
#include "components/UITheme.h"

namespace fui = freeink::ui;

void NewsFeedActivity::onEnter() {
  UiListActivity::onEnter();
  MarketStore::loadGateway(gateway);
  refresh();
}

void NewsFeedActivity::onExit() {
  Activity::onExit();
  rows.clear();
  values.clear();
  subtitles.clear();
  articles.clear();
}

void NewsFeedActivity::refresh() {
  NewsGatewayClient client;
  const NewsFeedResponse response = client.fetch(gateway, feed.url, 15);

  if (!response.ok) {
    error = response.error;
    articles.clear();
  } else {
    error.clear();
    articles = response.articles;
    if (!response.feedTitle.empty()) feed.name = response.feedTitle;
  }

  rebuildRows();
  requestUpdate();
}

void NewsFeedActivity::rebuildRows() {
  rows.clear();
  values.clear();
  subtitles.clear();

  values.reserve(articles.size() + 1);
  subtitles.reserve(articles.size() + 1);
  rows.reserve(articles.size() + 1);

  values.push_back("Online");
  subtitles.push_back(feed.url);
  fui::ListItem refreshRow;
  refreshRow.label = "Odswiez kanal";
  refreshRow.value = values.back().c_str();
  refreshRow.subtitle = subtitles.back().c_str();
  refreshRow.actionValue = 0;
  rows.push_back(refreshRow);

  for (size_t i = 0; i < articles.size(); ++i) {
    const bool favorite = NewsStore::isFavorite(articles[i].link);

    std::string value = favorite ? "*" : "";
    if (!articles[i].published.empty()) {
      if (!value.empty()) value += " | ";
      value += articles[i].published;
    }
    values.push_back(std::move(value));

    std::string subtitle = articles[i].source;
    if (subtitle.empty()) subtitle = feed.name;
    subtitles.push_back(std::move(subtitle));

    fui::ListItem row;
    row.label = articles[i].title.c_str();
    row.value = values.back().c_str();
    row.subtitle = subtitles.back().c_str();
    row.actionValue = static_cast<int16_t>(i + 1);
    rows.push_back(row);
  }

  header = feed.name.empty() ? "News Terminal" : feed.name;
  if (!error.empty()) header += " !";
}

void NewsFeedActivity::openArticle(const int index) {
  if (index < 0 || index >= static_cast<int>(articles.size())) return;

  if (!NewsStore::writeArticleText(articles[index])) {
    error = "Blad zapisu artykulu";
    rebuildRows();
    requestUpdate();
    return;
  }

  app.clearTapFlash();
  activityManager.goToReader(NewsStore::ARTICLE_PATH);
}

void NewsFeedActivity::toggleFavoriteAt(const int index) {
  if (index < 0 || index >= static_cast<int>(articles.size())) return;

  bool nowFavorite = false;
  if (!NewsStore::toggleFavorite(articles[index], &nowFavorite)) {
    error = "Blad Ulubionych";
  } else {
    error.clear();
  }
  rebuildRows();
  requestUpdate();
}

void NewsFeedActivity::activateIndex(const int index) {
  if (index == 0) {
    refresh();
    return;
  }
  openArticle(index - 1);
}

void NewsFeedActivity::onRowLongPress(const int index) {
  if (index <= 0) return;
  toggleFavoriteAt(index - 1);
}

void NewsFeedActivity::buildScreen(UiScreen& screen) {
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

void NewsFeedActivity::drawFooter() {
  const auto labels = mappedInput.mapLabels(
      "Back", "Open | Hold: *", tr(STR_DIR_UP), tr(STR_DIR_DOWN));
  GUI.drawButtonHints(renderer, labels.btn1, labels.btn2, labels.btn3, labels.btn4);
}
