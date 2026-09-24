#pragma once

#include <string>
#include <vector>

#include "ReaderDashboardStore.h"
#include "activities/UiListActivity.h"

class ReaderDashboardActivity final : public UiListActivity {
 private:
  ReaderDashboardSnapshot snapshot;
  std::string header = "Reader Dashboard";

  std::vector<std::string> labels;
  std::vector<std::string> values;
  std::vector<std::string> subtitles;
  std::vector<std::string> actions;
  std::vector<freeink::ui::ListItem> rows;

  void reload();
  void rebuildRows();
  void addRow(const std::string& action,
              std::string label,
              std::string value,
              std::string subtitle = {});
  void continueReading();
  void openLibrary();

 protected:
  int listCount() const override { return static_cast<int>(rows.size()); }
  void buildScreen(UiScreen& screen) override;
  void activateIndex(int index) override;
  const char* headerTitle() const override { return header.c_str(); }

 public:
  ReaderDashboardActivity(GfxRenderer& renderer, MappedInputManager& mappedInput)
      : UiListActivity("ReaderDashboard", renderer, mappedInput) {}

  void onEnter() override;
  void onExit() override;
};
