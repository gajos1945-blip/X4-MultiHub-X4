#include "NewsActivity.h"

#include <GfxRenderer.h>
#include <I18n.h>

#include <memory>

#include "MarketStore.h"
#include "NewsFavoritesActivity.h"
#include "NewsFeedActivity.h"
#include "activities/util/KeyboardEntryActivity.h"
#include "components/UITheme.h"

namespace fui = freeink::ui;

void NewsActivity::onEnter() {
  UiListActivity::onEnter();
  reload();
}

void NewsActivity::onExit() {
  Activity::onExit();
  rows.clear();
  values.clear();
  feeds.clear();
}

void NewsActivity::reload() {
  MarketStore::loadGateway(gateway);
  feeds.clear();
  NewsStore::loadFeeds(feeds);
  rebuildRows();
}

void NewsActivity::rebuildRows() {
  rows.clear();
  values.clear();

  rows.reserve(feeds.size() + CONTROL_ROWS);
  values.reserve(feeds.size() + CONTROL_ROWS);

  values.push_back(gateway.empty() ? "USTAW http://IP:8788" : gateway);
  fui::ListItem gatewayRow;
  gatewayRow.label = "X4 Data Gateway";
  gatewayRow.value = values.back().c_str();
  gatewayRow.actionValue = 0;
  rows.push_back(gatewayRow);

  values.push_back("+");
  fui::ListItem addRow;
  addRow.label = "Dodaj zrodlo RSS/Atom";
  addRow.value = values.back().c_str();
  addRow.actionValue = 1;
  rows.push_back(addRow);

  std::vector<NewsArticle> favorites;
  NewsStore::loadFavorites(favorites);
  values.push_back(std::to_string(favorites.size()));
  fui::ListItem favoriteRow;
  favoriteRow.label = "Ulubione wiadomosci";
  favoriteRow.value = values.back().c_str();
  favoriteRow.actionValue = 2;
  rows.push_back(favoriteRow);

  for (size_t i = 0; i < feeds.size(); ++i) {
    values.push_back("RSS/Atom");
    fui::ListItem row;
    row.label = feeds[i].name.c_str();
    row.value = values.back().c_str();
    row.subtitle = feeds[i].url.c_str();
    row.actionValue = static_cast<int16_t>(i + CONTROL_ROWS);
    rows.push_back(row);
  }

  header = "News Terminal (";
  header += std::to_string(feeds.size());
  header += ")";
}

void NewsActivity::editGateway() {
  startActivityForResult(
      std::make_unique<KeyboardEntryActivity>(
          renderer, mappedInput, "Gateway http://IP:8788",
          gateway, 160, InputType::Text),
      [this](const ActivityResult& result) {
        if (!result.isCancelled) {
          const std::string value = std::get<KeyboardResult>(result.data).text;
          if (MarketStore::saveGateway(value)) {
            gateway = value;
            header = "News Terminal";
          } else {
            header = "Niepoprawny gateway";
          }
        }
        rebuildRows();
        requestUpdate();
      });
}

void NewsActivity::addFeed() {
  startActivityForResult(
      std::make_unique<KeyboardEntryActivity>(
          renderer, mappedInput, "Nazwa zrodla", "", 80, InputType::Text),
      [this](const ActivityResult& nameResult) {
        if (nameResult.isCancelled) return;
        const std::string name = std::get<KeyboardResult>(nameResult.data).text;
        if (!NewsStore::validFeedName(name)) {
          header = "Niepoprawna nazwa zrodla";
          requestUpdate();
          return;
        }

        startActivityForResult(
            std::make_unique<KeyboardEntryActivity>(
                renderer, mappedInput, "URL RSS/Atom http(s)://",
                "", 500, InputType::Text),
            [this, name](const ActivityResult& urlResult) {
              if (!urlResult.isCancelled) {
                const std::string url = std::get<KeyboardResult>(urlResult.data).text;
                if (NewsStore::addFeed(NewsFeed{name, url})) {
                  reload();
                } else {
                  header = "Nie mozna dodac zrodla";
                }
              }
              requestUpdate();
            });
      });
}

void NewsActivity::openFavorites() {
  startActivityForResult(
      std::make_unique<NewsFavoritesActivity>(renderer, mappedInput),
      [this](const ActivityResult&) {
        reload();
        requestUpdate();
      });
}

void NewsActivity::openFeed(const int index) {
  if (index < 0 || index >= static_cast<int>(feeds.size())) return;

  startActivityForResult(
      std::make_unique<NewsFeedActivity>(
          renderer, mappedInput, feeds[index]),
      [this](const ActivityResult&) {
        reload();
        requestUpdate();
      });
}

void NewsActivity::removeFeedAt(const int index) {
  if (index < 0 || index >= static_cast<int>(feeds.size())) return;

  if (!NewsStore::removeFeed(feeds[index].url)) {
    header = "Blad usuwania zrodla";
    requestUpdate();
    return;
  }
  reload();
  requestUpdate();
}

void NewsActivity::activateIndex(const int index) {
  switch (index) {
    case 0: editGateway(); return;
    case 1: addFeed(); return;
    case 2: openFavorites(); return;
    default: openFeed(index - CONTROL_ROWS); return;
  }
}

void NewsActivity::onRowLongPress(const int index) {
  if (index < CONTROL_ROWS) return;
  removeFeedAt(index - CONTROL_ROWS);
}

void NewsActivity::buildScreen(UiScreen& screen) {
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

void NewsActivity::drawFooter() {
  const auto labels = mappedInput.mapLabels(
      "Back", "Open | Hold feed: remove", tr(STR_DIR_UP), tr(STR_DIR_DOWN));
  GUI.drawButtonHints(renderer, labels.btn1, labels.btn2, labels.btn3, labels.btn4);
}
