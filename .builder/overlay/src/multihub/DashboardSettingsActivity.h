#pragma once

#include <string>
#include <vector>

#include "DashboardStore.h"
#include "activities/UiListActivity.h"

class DashboardSettingsActivity final : public UiListActivity {
 private:
  DashboardConfig& config;
  std::vector<std::string> values;
  std::vector<freeink::ui::ListItem> rows;
  std::string header = "Uklad Dashboard";

  void rebuildRows();
  bool visible(const std::string& card) const;
  void toggle(const std::string& card);
  void moveUp(size_t index);

 protected:
  int listCount() const override { return static_cast<int>(rows.size()); }
  void buildScreen(UiScreen& screen) override;
  void activateIndex(int index) override;
  void onRowLongPress(int index) override;
  const char* headerTitle() const override { return header.c_str(); }
  void drawFooter() override;

 public:
  DashboardSettingsActivity(GfxRenderer& renderer,
                            MappedInputManager& mappedInput,
                            DashboardConfig& config)
      : UiListActivity("DashboardSettings", renderer, mappedInput, true),
        config(config) {}

  void onEnter() override;
};
