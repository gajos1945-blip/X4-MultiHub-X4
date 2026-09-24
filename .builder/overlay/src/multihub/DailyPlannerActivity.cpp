#include "DailyPlannerActivity.h"

#include <GfxRenderer.h>
#include <I18n.h>

#include <algorithm>
#include <memory>

#include "PlannerTaskActionsActivity.h"
#include "TimeService.h"
#include "activities/util/KeyboardEntryActivity.h"
#include "components/UITheme.h"

namespace fui = freeink::ui;

namespace {
const char* priorityShort(const uint8_t priority) {
  switch (priority) {
    case 1: return "P1";
    case 3: return "P3";
    default: return "P2";
  }
}
}  // namespace

void DailyPlannerActivity::onEnter() {
  UiListActivity::onEnter();
  reload();
}

void DailyPlannerActivity::onExit() {
  Activity::onExit();
  rows.clear();
  rowLabels.clear();
  rowValues.clear();
  rowSubtitles.clear();
  visible.clear();
  tasks.clear();
}

void DailyPlannerActivity::reload() {
  tasks.clear();
  PlannerStore::load(tasks);
  PlannerStore::loadActiveDate(activeDate);

  std::string todayDate;
  if (TimeService::applyTodayToPlanner(&todayDate)) activeDate = todayDate;

  rebuildVisible();
}

const char* DailyPlannerActivity::viewName() const {
  switch (viewMode) {
    case 1: return "Otwarte";
    case 2: return "Wykonane";
    default: return "Wszystkie";
  }
}

void DailyPlannerActivity::rebuildVisible() {
  visible.clear();

  if (PlannerStore::validDate(activeDate)) {
    for (int i = 0; i < static_cast<int>(tasks.size()); ++i) {
      const auto& task = tasks[i];
      if (task.date != activeDate) continue;
      if (viewMode == 1 && task.done) continue;
      if (viewMode == 2 && !task.done) continue;
      visible.push_back(i);
    }
  }

  std::sort(visible.begin(), visible.end(), [&](const int ai, const int bi) {
    const auto& a = tasks[ai];
    const auto& b = tasks[bi];
    if (a.done != b.done) return !a.done;
    if (a.priority != b.priority) return a.priority < b.priority;
    return a.id < b.id;
  });

  rebuildRows();
  if (nav.selected >= listCount()) nav.selected = listCount() - 1;
  if (nav.selected < 0) nav.selected = 0;
  nav.top = 0;
  nav.follow(listCount());
}

void DailyPlannerActivity::rebuildRows() {
  rows.clear();
  rowLabels.clear();
  rowValues.clear();
  rowSubtitles.clear();

  rows.reserve(visible.size() + CONTROL_ROWS);
  rowLabels.reserve(visible.size());
  rowValues.reserve(visible.size());
  rowSubtitles.reserve(visible.size());

  fui::ListItem date;
  date.label = "Dzien";
  date.value = PlannerStore::validDate(activeDate) ? activeDate.c_str() : "USTAW DATE";
  date.actionValue = 0;
  rows.push_back(date);

  fui::ListItem prev;
  prev.label = "Poprzedni dzien";
  prev.value = "<";
  prev.actionValue = 1;
  rows.push_back(prev);

  fui::ListItem next;
  next.label = "Nastepny dzien";
  next.value = ">";
  next.actionValue = 2;
  rows.push_back(next);

  fui::ListItem add;
  add.label = "Dodaj zadanie";
  add.value = "+";
  add.actionValue = 3;
  rows.push_back(add);

  fui::ListItem view;
  view.label = "Widok";
  view.value = viewName();
  view.actionValue = 4;
  rows.push_back(view);

  for (const int index : visible) {
    const auto& task = tasks[index];
    rowLabels.push_back(task.title);
    rowValues.push_back(task.done ? "[x]" : "[ ]");

    std::string subtitle = priorityShort(task.priority);
    if (!task.note.empty()) {
      subtitle += " | ";
      subtitle += task.note;
    }
    rowSubtitles.push_back(std::move(subtitle));
  }

  for (size_t i = 0; i < visible.size(); ++i) {
    fui::ListItem row;
    row.label = rowLabels[i].c_str();
    row.value = rowValues[i].c_str();
    row.subtitle = rowSubtitles[i].c_str();
    row.actionValue = static_cast<int16_t>(i + CONTROL_ROWS);
    rows.push_back(row);
  }

  header = "Daily Planner (";
  header += std::to_string(visible.size());
  header += ")";
}

void DailyPlannerActivity::editActiveDate() {
  const std::string initial = PlannerStore::validDate(activeDate) ? activeDate : "";

  startActivityForResult(
      std::make_unique<KeyboardEntryActivity>(
          renderer, mappedInput, "Data YYYY-MM-DD", initial, 10, InputType::Text),
      [this](const ActivityResult& result) {
        if (!result.isCancelled) {
          const std::string value = std::get<KeyboardResult>(result.data).text;
          if (PlannerStore::validDate(value) && PlannerStore::saveActiveDate(value)) {
            activeDate = value;
          } else {
            header = "Daily Planner - NIEPOPRAWNA DATA";
          }
        }
        rebuildVisible();
        requestUpdate();
      });
}

void DailyPlannerActivity::shiftActiveDate(const int delta) {
  if (!PlannerStore::validDate(activeDate)) {
    header = "Najpierw ustaw date";
    requestUpdate();
    return;
  }

  std::string shifted;
  if (PlannerStore::shiftDate(activeDate, delta, shifted) &&
      PlannerStore::saveActiveDate(shifted)) {
    activeDate = shifted;
    rebuildVisible();
  } else {
    header = "Blad zmiany daty";
  }
  requestUpdate();
}

void DailyPlannerActivity::addTask() {
  if (!PlannerStore::validDate(activeDate)) {
    header = "Najpierw ustaw date";
    requestUpdate();
    return;
  }

  startActivityForResult(
      std::make_unique<KeyboardEntryActivity>(
          renderer, mappedInput, "Nowe zadanie", "", 96, InputType::Text),
      [this](const ActivityResult& result) {
        if (!result.isCancelled) {
          const std::string title = std::get<KeyboardResult>(result.data).text;
          if (!title.empty() && tasks.size() < PlannerStore::MAX_TASKS) {
            PlannerTask task;
            task.id = PlannerStore::nextId(tasks);
            task.date = activeDate;
            task.title = title;
            task.priority = 2;
            tasks.push_back(std::move(task));
            if (!PlannerStore::save(tasks)) {
              tasks.pop_back();
              header = "BLAD ZAPISU ZADANIA";
            }
          }
        }
        rebuildVisible();
        requestUpdate();
      });
}

void DailyPlannerActivity::cycleView() {
  viewMode = (viewMode + 1) % 3;
  rebuildVisible();
  nav.selected = 4;
  requestUpdate();
}

void DailyPlannerActivity::toggleTaskAt(const int visibleIndex) {
  if (visibleIndex < 0 || visibleIndex >= static_cast<int>(visible.size())) return;
  PlannerTask& task = tasks[visible[visibleIndex]];
  task.done = !task.done;
  if (!PlannerStore::save(tasks)) {
    task.done = !task.done;
    header = "BLAD ZAPISU ZADANIA";
  }
  rebuildVisible();
  requestUpdate();
}

void DailyPlannerActivity::openTaskActions(const int visibleIndex) {
  if (visibleIndex < 0 || visibleIndex >= static_cast<int>(visible.size())) return;
  const uint32_t id = tasks[visible[visibleIndex]].id;

  startActivityForResult(
      std::make_unique<PlannerTaskActionsActivity>(
          renderer, mappedInput, tasks, id),
      [this](const ActivityResult&) {
        rebuildVisible();
        requestUpdate();
      });
}

void DailyPlannerActivity::activateIndex(const int index) {
  switch (index) {
    case 0: editActiveDate(); return;
    case 1: shiftActiveDate(-1); return;
    case 2: shiftActiveDate(1); return;
    case 3: addTask(); return;
    case 4: cycleView(); return;
    default: break;
  }
  toggleTaskAt(index - CONTROL_ROWS);
}

void DailyPlannerActivity::onRowLongPress(const int index) {
  if (index < CONTROL_ROWS) return;
  openTaskActions(index - CONTROL_ROWS);
}

void DailyPlannerActivity::buildScreen(UiScreen& screen) {
  const auto& metrics = UITheme::getInstance().getMetrics();
  screen.setContentMarginFromScreen(
      fui::Insets{static_cast<int16_t>(metrics.topPadding + metrics.headerHeight), 0,
                  static_cast<int16_t>(metrics.buttonHintsHeight), 0});
  screen.spacer(static_cast<int16_t>(metrics.verticalSpacing));

  fui::ListProps props;
  props.items = rows.data();
  props.count = static_cast<uint16_t>(rows.size());
  props.action = ACTION_ROW;
  props.inputMask = fui::InputTouch | fui::InputLongPress;
  fui::TextStyle label = screen.theme().smallText;
  label.bold = true;
  props.labelText = label;
  props.valueInset = 8;
  syncListViewport(screen, props, true);
  screen.list(props);
}

void DailyPlannerActivity::drawFooter() {
  const auto labels = mappedInput.mapLabels(
      "Back",
      nav.selected >= CONTROL_ROWS ? "Toggle / Hold: edit" : "Select",
      tr(STR_DIR_UP), tr(STR_DIR_DOWN));
  GUI.drawButtonHints(renderer, labels.btn1, labels.btn2, labels.btn3, labels.btn4);
}
