#pragma once

#include <string>
#include <vector>

#include "PlannerStore.h"
#include "activities/UiListActivity.h"

class DailyPlannerActivity final : public UiListActivity {
 private:
  static constexpr int CONTROL_ROWS = 5;

  std::vector<PlannerTask> tasks;
  std::vector<int> visible;
  std::vector<std::string> rowLabels;
  std::vector<std::string> rowValues;
  std::vector<std::string> rowSubtitles;
  std::vector<freeink::ui::ListItem> rows;

  std::string activeDate;
  std::string header = "Daily Planner";
  int viewMode = 0;  // 0=all, 1=open, 2=done

  void reload();
  void rebuildVisible();
  void rebuildRows();
  void editActiveDate();
  void shiftActiveDate(int delta);
  void addTask();
  void cycleView();
  void toggleTaskAt(int visibleIndex);
  void openTaskActions(int visibleIndex);
  const char* viewName() const;

 protected:
  int listCount() const override { return static_cast<int>(rows.size()); }
  void buildScreen(UiScreen& screen) override;
  void activateIndex(int index) override;
  void onRowLongPress(int index) override;
  const char* headerTitle() const override { return header.c_str(); }
  void drawFooter() override;

 public:
  DailyPlannerActivity(GfxRenderer& renderer, MappedInputManager& mappedInput)
      : UiListActivity("DailyPlanner", renderer, mappedInput, true) {}

  void onEnter() override;
  void onExit() override;
};
