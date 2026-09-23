#pragma once

#include <string>
#include <vector>

#include "activities/UiListActivity.h"

struct ChecklistRow {
  size_t lineIndex = 0;
  std::string label;
  bool checked = false;
};

class ChecklistActivity final : public UiListActivity {
 private:
  static constexpr size_t MAX_FILE_BYTES = 16 * 1024;
  static constexpr size_t MAX_ITEMS = 80;

  std::string path;
  std::string header = "Checklist";
  std::vector<std::string> lines;
  std::vector<ChecklistRow> items;
  std::vector<std::string> values;
  std::vector<freeink::ui::ListItem> rows;

  bool loadChecklist();
  bool saveChecklist();
  void rebuildRows();

 protected:
  int listCount() const override { return static_cast<int>(rows.size()); }
  void buildScreen(UiScreen& screen) override;
  void activateIndex(int index) override;
  bool handleButtons() override;
  const char* headerTitle() const override { return header.c_str(); }

 public:
  ChecklistActivity(GfxRenderer& renderer, MappedInputManager& mappedInput,
                    std::string path)
      : UiListActivity("Checklist", renderer, mappedInput),
        path(std::move(path)) {}

  void onEnter() override;
  void onExit() override;
};
