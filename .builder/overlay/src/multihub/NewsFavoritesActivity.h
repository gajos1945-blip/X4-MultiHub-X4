#pragma once

#include <string>
#include <vector>

#include "NewsStore.h"
#include "activities/UiListActivity.h"

class NewsFavoritesActivity final : public UiListActivity {
 private:
  std::vector<NewsArticle> items;
  std::vector<std::string> values;
  std::vector<std::string> subtitles;
  std::vector<freeink::ui::ListItem> rows;
  std::string header = "Ulubione wiadomosci";

  void reload();
  void openArticle(int index);
  void removeAt(int index);

 protected:
  int listCount() const override { return static_cast<int>(rows.size()); }
  void buildScreen(UiScreen& screen) override;
  void activateIndex(int index) override;
  void onRowLongPress(int index) override;
  const char* headerTitle() const override { return header.c_str(); }
  void drawFooter() override;

 public:
  NewsFavoritesActivity(GfxRenderer& renderer, MappedInputManager& mappedInput)
      : UiListActivity("NewsFavorites", renderer, mappedInput, true) {}

  void onEnter() override;
  void onExit() override;
};
