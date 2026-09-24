#pragma once

#include <string>
#include <vector>

#include "NewsStore.h"
#include "activities/UiListActivity.h"

class NewsActivity final : public UiListActivity {
 private:
  static constexpr int CONTROL_ROWS = 3;

  std::vector<NewsFeed> feeds;
  std::vector<std::string> values;
  std::vector<freeink::ui::ListItem> rows;

  std::string gateway;
  std::string header = "News Terminal";

  void reload();
  void rebuildRows();
  void editGateway();
  void addFeed();
  void openFavorites();
  void openFeed(int index);
  void removeFeedAt(int index);

 protected:
  int listCount() const override { return static_cast<int>(rows.size()); }
  void buildScreen(UiScreen& screen) override;
  void activateIndex(int index) override;
  void onRowLongPress(int index) override;
  const char* headerTitle() const override { return header.c_str(); }
  void drawFooter() override;

 public:
  NewsActivity(GfxRenderer& renderer, MappedInputManager& mappedInput)
      : UiListActivity("NewsTerminal", renderer, mappedInput, true) {}

  void onEnter() override;
  void onExit() override;
};
