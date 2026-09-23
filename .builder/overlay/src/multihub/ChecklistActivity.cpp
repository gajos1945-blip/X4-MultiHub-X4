#include "ChecklistActivity.h"

#include <HalStorage.h>
#include <GfxRenderer.h>

#include <sstream>

#include "MappedInputManager.h"
#include "components/UITheme.h"

namespace fui = freeink::ui;

namespace {
std::string filenameStem(const std::string& path) {
  const size_t slash = path.find_last_of('/');
  std::string name = slash == std::string::npos ? path : path.substr(slash + 1);
  const size_t dot = name.find_last_of('.');
  if (dot != std::string::npos) name.resize(dot);
  return name.empty() ? "Checklist" : name;
}

bool parseChecklistLine(const std::string& line, bool& checked, std::string& label) {
  if (line.size() < 4 || line[0] != '[' || line[2] != ']') return false;
  if (line[1] == ' ') {
    checked = false;
  } else if (line[1] == 'x' || line[1] == 'X') {
    checked = true;
  } else {
    return false;
  }

  size_t start = 3;
  while (start < line.size() && line[start] == ' ') ++start;
  label = line.substr(start);
  return !label.empty();
}
}  // namespace

bool ChecklistActivity::loadChecklist() {
  lines.clear();
  items.clear();

  HalFile file;
  if (!Storage.openFileForRead("Checklist", path, file)) return false;
  if (static_cast<size_t>(file.size()) > MAX_FILE_BYTES) {
    file.close();
    return false;
  }

  std::string current;
  current.reserve(128);
  while (file.available()) {
    const int ch = file.read();
    if (ch < 0) break;

    if (ch == '\r') continue;
    if (ch == '\n') {
      lines.push_back(current);
      current.clear();
      continue;
    }
    current.push_back(static_cast<char>(ch));
  }
  if (!current.empty()) lines.push_back(current);
  file.close();

  for (size_t i = 0; i < lines.size() && items.size() < MAX_ITEMS; ++i) {
    bool checked = false;
    std::string label;
    if (!parseChecklistLine(lines[i], checked, label)) continue;

    ChecklistRow item;
    item.lineIndex = i;
    item.label = std::move(label);
    item.checked = checked;
    items.push_back(std::move(item));
  }

  header = filenameStem(path);
  return true;
}

bool ChecklistActivity::saveChecklist() {
  std::string out;
  out.reserve(1024);
  for (size_t i = 0; i < lines.size(); ++i) {
    out += lines[i];
    if (i + 1 < lines.size()) out += '\n';
  }
  if (out.size() > MAX_FILE_BYTES) return false;

  const std::string temp = path + ".tmp";
  if (!Storage.writeFile(temp.c_str(), String(out.c_str()))) return false;

  Storage.remove(path.c_str());
  if (!Storage.rename(temp.c_str(), path.c_str())) {
    Storage.remove(temp.c_str());
    return false;
  }
  return true;
}

void ChecklistActivity::rebuildRows() {
  rows.clear();
  values.clear();
  rows.reserve(items.size());
  values.reserve(items.size());

  for (const auto& item : items) {
    values.push_back(item.checked ? "[x]" : "[ ]");
  }

  for (size_t i = 0; i < items.size(); ++i) {
    fui::ListItem row;
    row.label = items[i].label.c_str();
    row.value = values[i].c_str();
    row.actionValue = static_cast<int16_t>(i);
    rows.push_back(row);
  }

  if (rows.empty()) header += " - brak pozycji";
}

void ChecklistActivity::onEnter() {
  UiListActivity::onEnter();
  if (!loadChecklist()) {
    header = "Checklist - BLAD PLIKU";
    rows.clear();
    requestUpdate();
    return;
  }
  rebuildRows();
}

void ChecklistActivity::onExit() {
  Activity::onExit();
  rows.clear();
  values.clear();
  items.clear();
  lines.clear();
}

void ChecklistActivity::activateIndex(const int index) {
  if (index < 0 || index >= static_cast<int>(items.size())) return;

  auto& item = items[index];
  if (item.lineIndex >= lines.size()) return;

  item.checked = !item.checked;
  if (lines[item.lineIndex].size() < 3) return;
  lines[item.lineIndex][1] = item.checked ? 'x' : ' ';

  if (!saveChecklist()) {
    item.checked = !item.checked;
    lines[item.lineIndex][1] = item.checked ? 'x' : ' ';
    header = "Checklist - BLAD ZAPISU";
  }
  rebuildRows();
  requestUpdate();
}

bool ChecklistActivity::handleButtons() {
  if (mappedInput.wasReleased(MappedInputManager::Button::Back)) {
    finish();
    return true;
  }
  return false;
}

void ChecklistActivity::buildScreen(UiScreen& screen) {
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
