#include "PlannerTaskActionsActivity.h"

#include <GfxRenderer.h>

#include <algorithm>
#include <memory>

#include "activities/util/KeyboardEntryActivity.h"
#include "components/UITheme.h"

namespace fui = freeink::ui;

namespace {
const char* priorityName(const uint8_t priority) {
  switch (priority) {
    case 1: return "Wysoki";
    case 3: return "Niski";
    default: return "Normalny";
  }
}
}  // namespace

PlannerTask* PlannerTaskActionsActivity::findTask() {
  for (auto& task : tasks) {
    if (task.id == taskId) return &task;
  }
  return nullptr;
}

void PlannerTaskActionsActivity::onEnter() {
  UiListActivity::onEnter();
  rebuildRows();
}

void PlannerTaskActionsActivity::rebuildRows() {
  PlannerTask* task = findTask();
  if (!task) {
    header = "Zadanie - BRAK";
    return;
  }

  header = task->title;

  values[0] = task->done ? "Wykonane" : "Otwarte";
  values[1] = priorityName(task->priority);
  values[2] = task->title;
  values[3] = task->note.empty() ? "(brak)" : task->note;
  values[4] = task->date;
  values[5] = "Usun trwale";

  const char* labels[ROWS] = {
      "Status", "Priorytet", "Tytul", "Notatka", "Data", "Usun zadanie"
  };

  for (int i = 0; i < ROWS; ++i) {
    rows[i] = {};
    rows[i].label = labels[i];
    rows[i].value = values[i].c_str();
    rows[i].actionValue = static_cast<int16_t>(i);
  }
}

void PlannerTaskActionsActivity::toggleDone() {
  PlannerTask* task = findTask();
  if (!task) return;
  task->done = !task->done;
  PlannerStore::save(tasks);
  rebuildRows();
  requestUpdate();
}

void PlannerTaskActionsActivity::cyclePriority() {
  PlannerTask* task = findTask();
  if (!task) return;
  task->priority = task->priority >= 3 ? 1 : task->priority + 1;
  PlannerStore::save(tasks);
  rebuildRows();
  requestUpdate();
}

void PlannerTaskActionsActivity::editTitle() {
  PlannerTask* task = findTask();
  if (!task) return;

  startActivityForResult(
      std::make_unique<KeyboardEntryActivity>(
          renderer, mappedInput, "Tytul zadania", task->title, 96, InputType::Text),
      [this](const ActivityResult& result) {
        if (!result.isCancelled) {
          PlannerTask* current = findTask();
          const std::string value = std::get<KeyboardResult>(result.data).text;
          if (current && !value.empty()) {
            current->title = value;
            PlannerStore::save(tasks);
          }
        }
        rebuildRows();
        requestUpdate();
      });
}

void PlannerTaskActionsActivity::editNote() {
  PlannerTask* task = findTask();
  if (!task) return;

  startActivityForResult(
      std::make_unique<KeyboardEntryActivity>(
          renderer, mappedInput, "Notatka do zadania", task->note, 180, InputType::Text),
      [this](const ActivityResult& result) {
        if (!result.isCancelled) {
          PlannerTask* current = findTask();
          if (current) {
            current->note = std::get<KeyboardResult>(result.data).text;
            PlannerStore::save(tasks);
          }
        }
        rebuildRows();
        requestUpdate();
      });
}

void PlannerTaskActionsActivity::moveDate() {
  PlannerTask* task = findTask();
  if (!task) return;

  startActivityForResult(
      std::make_unique<KeyboardEntryActivity>(
          renderer, mappedInput, "Data YYYY-MM-DD", task->date, 10, InputType::Text),
      [this](const ActivityResult& result) {
        if (!result.isCancelled) {
          PlannerTask* current = findTask();
          const std::string value = std::get<KeyboardResult>(result.data).text;
          if (current && PlannerStore::validDate(value)) {
            current->date = value;
            PlannerStore::save(tasks);
          }
        }
        rebuildRows();
        requestUpdate();
      });
}

void PlannerTaskActionsActivity::deleteTask() {
  tasks.erase(
      std::remove_if(tasks.begin(), tasks.end(),
                     [&](const PlannerTask& task) { return task.id == taskId; }),
      tasks.end());
  PlannerStore::save(tasks);
  finish();
}

void PlannerTaskActionsActivity::activateIndex(const int index) {
  switch (index) {
    case 0: toggleDone(); break;
    case 1: cyclePriority(); break;
    case 2: editTitle(); break;
    case 3: editNote(); break;
    case 4: moveDate(); break;
    case 5: deleteTask(); break;
    default: break;
  }
}

void PlannerTaskActionsActivity::buildScreen(UiScreen& screen) {
  const auto& metrics = UITheme::getInstance().getMetrics();
  screen.setContentMarginFromScreen(
      fui::Insets{static_cast<int16_t>(metrics.topPadding + metrics.headerHeight), 0,
                  static_cast<int16_t>(metrics.buttonHintsHeight), 0});
  screen.spacer(static_cast<int16_t>(metrics.verticalSpacing));

  fui::ListProps props;
  props.items = rows.data();
  props.count = static_cast<uint16_t>(rows.size());
  props.action = ACTION_ROW;
  props.inputMask = fui::InputTouch;
  props.valueInset = 8;
  props.labelText = screen.theme().smallText;
  props.labelText.maxLines = 2;
  syncListViewport(screen, props);
  screen.list(props);
}
