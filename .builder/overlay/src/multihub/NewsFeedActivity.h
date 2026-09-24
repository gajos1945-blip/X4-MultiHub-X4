#pragma once

#include <string>
#include <utility>
#include <vector>

#include "NewsGatewayClient.h"
#include "NewsStore.h"
#include "activities/UiListActivity.h"

class NewsFeedActivity final : public UiListActivity {
 private:
  NewsFeed feed;
  std::string gateway;
  std::string header = "News Terminal";
  std::string error;
  std::vector<NewsArticle> articles;
  std::vector<std::string> values;
  std::vector<std::string> subtitles;
  std::vector<freeink::ui::ListItem> rows;

  void refresh();
  void rebuildRows();
  void openArticle(int index);
  void toggleFavoriteAt(int index);

 protected:
  int listCount() const override { return static_cast<int>(rows.size()); }
  void buildScreen(UiScreen& screen) override;
  void activateIndex(int index) override;
  void onRowLongPress(int index) override;
  const char* headerTitle() const override { return header.c_str(); }
  void drawFooter() override;

 public:
  NewsFeedActivity(GfxRenderer& renderer,
                   MappedInputManager& mappedInput,
                   NewsFeed feed)
      : UiListActivity("NewsFeed", renderer, mappedInput, true),
        feed(std::move(feed)) {}

  void onEnter() override;
  void onExit() override;
};
