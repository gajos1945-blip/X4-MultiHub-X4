#pragma once

#include <array>
#include <string>
#include <vector>

#include "PlannerStore.h"
#include "activities/UiListActivity.h"

class PlannerTaskActionsActivity final : public UiListActivity {
 private:
  static constexpr int ROWS = 6;

  std::vector<PlannerTask>& tasks;
  uint32_t taskId;
  std::array<std::string, ROWS> values{};
  std::array<freeink::ui::ListItem, ROWS> rows{};
  std::string header = "Zadanie";

  PlannerTask* findTask();
  void rebuildRows();
  void editTitle();
  void editNote();
  void cyclePriority();
  void toggleDone();
  void moveDate();
  void deleteTask();

 protected:
  int listCount() const override { return ROWS; }
  void buildScreen(UiScreen& screen) override;
  void activateIndex(int index) override;
  const char* headerTitle() const override { return header.c_str(); }

 public:
  PlannerTaskActionsActivity(GfxRenderer& renderer, MappedInputManager& mappedInput,
                             std::vector<PlannerTask>& tasks, uint32_t taskId)
      : UiListActivity("PlannerTaskActions", renderer, mappedInput),
        tasks(tasks), taskId(taskId) {}

  void onEnter() override;
};
