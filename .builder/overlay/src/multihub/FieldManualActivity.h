#pragma once

#include <string>
#include <vector>

#include "FieldManualStore.h"
#include "activities/UiListActivity.h"

class FieldManualActivity final : public UiListActivity {
 private:
  static constexpr int CONTROL_ROWS = 3;
  static constexpr unsigned long ACTION_HOLD_MS = 900;

  std::vector<FieldManualEntry> entries;
  std::vector<int> visible;
  std::vector<std::string> categories;
  std::vector<std::string> rowLabels;
  std::vector<std::string> rowSubtitles;
  std::vector<freeink::ui::ListItem> rowItems;

  std::string searchQuery;
  std::string header = "Field Manual";
  int filterIndex = 0;  // 0=all, 1=favorites, 2+=category
  bool indexLimitReached = false;

  void reloadIndex();
  void rebuildCategories();
  void rebuildVisible();
  void rebuildRows();
  void openSearch();
  void cycleFilter();
  void rebuildIndex();
  void toggleFavoriteAt(int visibleIndex);
  const char* filterName() const;

 protected:
  int listCount() const override {
    return static_cast<int>(visible.size()) + CONTROL_ROWS;
  }
  void buildScreen(UiScreen& screen) override;
  void activateIndex(int index) override;
  bool handleButtons() override;
  const char* headerTitle() const override { return header.c_str(); }
  void drawFooter() override;

 public:
  FieldManualActivity(GfxRenderer& renderer, MappedInputManager& mappedInput)
      : UiListActivity("FieldManual", renderer, mappedInput, true) {}

  void onEnter() override;
  void onExit() override;
  void onRowLongPress(int index) override;
};
