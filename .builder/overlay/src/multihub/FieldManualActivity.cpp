#include "FieldManualActivity.h"

#include <FsHelpers.h>
#include <GfxRenderer.h>
#include <I18n.h>

#include <algorithm>
#include <cctype>
#include <memory>

#include "ChecklistActivity.h"
#include "MappedInputManager.h"
#include "activities/ActivityManager.h"
#include "activities/util/KeyboardEntryActivity.h"
#include "components/UITheme.h"

namespace fui = freeink::ui;

namespace {
std::string asciiFold(const std::string& value) {
  std::string out = value;
  for (char& c : out) {
    const unsigned char u = static_cast<unsigned char>(c);
    if (u < 128) c = static_cast<char>(std::tolower(u));
  }
  return out;
}

bool matchesSearch(const FieldManualEntry& entry, const std::string& query) {
  if (query.empty()) return true;
  return asciiFold(entry.title).find(query) != std::string::npos ||
         asciiFold(entry.category).find(query) != std::string::npos ||
         asciiFold(entry.path).find(query) != std::string::npos;
}
}  // namespace

void FieldManualActivity::onEnter() {
  UiListActivity::onEnter();
  reloadIndex();
}

void FieldManualActivity::onExit() {
  Activity::onExit();
  rowItems.clear();
  rowLabels.clear();
  rowSubtitles.clear();
  visible.clear();
  categories.clear();
  entries.clear();
}

void FieldManualActivity::reloadIndex() {
  entries.clear();
  if (!FieldManualStore::load(entries)) {
    FieldManualStore::rebuild(entries);
  }

  indexLimitReached = entries.size() >= FieldManualStore::MAX_MANUALS;
  rebuildCategories();

  const int maxFilter = static_cast<int>(categories.size()) + 1;
  if (filterIndex > maxFilter) filterIndex = 0;
  rebuildVisible();
}

void FieldManualActivity::rebuildCategories() {
  categories.clear();
  for (const auto& entry : entries) {
    if (entry.category.empty()) continue;
    if (std::find(categories.begin(), categories.end(), entry.category) == categories.end()) {
      categories.push_back(entry.category);
    }
  }
  std::sort(categories.begin(), categories.end(),
            [](const std::string& a, const std::string& b) {
              return FsHelpers::naturalLess(a, b);
            });
}

const char* FieldManualActivity::filterName() const {
  if (filterIndex == 0) return "Wszystkie";
  if (filterIndex == 1) return "Ulubione";
  const int category = filterIndex - 2;
  if (category >= 0 && category < static_cast<int>(categories.size())) {
    return categories[category].c_str();
  }
  return "Wszystkie";
}

void FieldManualActivity::rebuildVisible() {
  visible.clear();
  const std::string query = asciiFold(searchQuery);

  for (int i = 0; i < static_cast<int>(entries.size()); ++i) {
    const auto& entry = entries[i];
    if (!matchesSearch(entry, query)) continue;

    if (filterIndex == 1 && !entry.favorite) continue;
    if (filterIndex >= 2) {
      const int category = filterIndex - 2;
      if (category < 0 || category >= static_cast<int>(categories.size())) continue;
      if (entry.category != categories[category]) continue;
    }
    visible.push_back(i);
  }

  std::sort(visible.begin(), visible.end(), [&](const int ai, const int bi) {
    const auto& a = entries[ai];
    const auto& b = entries[bi];
    if (a.category != b.category) return FsHelpers::naturalLess(a.category, b.category);
    return FsHelpers::naturalLess(a.title, b.title);
  });

  if (nav.selected >= listCount()) nav.selected = listCount() - 1;
  if (nav.selected < 0) nav.selected = 0;
  nav.top = 0;
  nav.follow(listCount());
  rebuildRows();
}

void FieldManualActivity::rebuildRows() {
  rowLabels.clear();
  rowSubtitles.clear();
  rowItems.clear();

  rowLabels.reserve(visible.size());
  rowSubtitles.reserve(visible.size());
  rowItems.reserve(visible.size() + CONTROL_ROWS);

  fui::ListItem search;
  search.label = "Szukaj";
  search.value = searchQuery.empty() ? "(wszystkie)" : searchQuery.c_str();
  search.actionValue = 0;
  rowItems.push_back(search);

  fui::ListItem filter;
  filter.label = "Widok";
  filter.value = filterName();
  filter.actionValue = 1;
  rowItems.push_back(filter);

  fui::ListItem rebuild;
  rebuild.label = "Przebuduj indeks";
  rebuild.value = "/Manuals";
  rebuild.actionValue = 2;
  rowItems.push_back(rebuild);

  for (const int index : visible) {
    const auto& entry = entries[index];
    rowLabels.push_back(entry.title);

    std::string subtitle = entry.category;
    if (entry.favorite) subtitle += " | *";
    if (entry.checklist) subtitle += " | Checklist";
    rowSubtitles.push_back(std::move(subtitle));
  }

  for (size_t i = 0; i < visible.size(); ++i) {
    fui::ListItem item;
    item.label = rowLabels[i].c_str();
    item.subtitle = rowSubtitles[i].c_str();
    item.actionValue = static_cast<int16_t>(i + CONTROL_ROWS);
    rowItems.push_back(item);
  }

  header = "Field Manual (";
  header += std::to_string(visible.size());
  header += ")";
  if (!searchQuery.empty()) header += " Q";
  if (indexLimitReached) header += " !";
}

void FieldManualActivity::openSearch() {
  startActivityForResult(
      std::make_unique<KeyboardEntryActivity>(
          renderer, mappedInput, "Szukaj: tytul/kategoria/sciezka",
          searchQuery, 80, InputType::Text),
      [this](const ActivityResult& result) {
        if (!result.isCancelled) {
          searchQuery = std::get<KeyboardResult>(result.data).text;
          nav.selected = 0;
          rebuildVisible();
        }
        requestUpdate();
      });
}

void FieldManualActivity::cycleFilter() {
  const int count = static_cast<int>(categories.size()) + 2;
  filterIndex = count > 0 ? (filterIndex + 1) % count : 0;
  nav.selected = 1;
  rebuildVisible();
  requestUpdate();
}

void FieldManualActivity::rebuildIndex() {
  if (FieldManualStore::rebuild(entries)) {
    header = "Field Manual - indeks OK";
  } else {
    header = "Field Manual - BLAD indeksu";
  }
  rebuildCategories();
  filterIndex = 0;
  rebuildVisible();
  requestUpdate();
}

void FieldManualActivity::toggleFavoriteAt(const int visibleIndex) {
  if (visibleIndex < 0 || visibleIndex >= static_cast<int>(visible.size())) return;
  const int entryIndex = visible[visibleIndex];
  if (entryIndex < 0 || entryIndex >= static_cast<int>(entries.size())) return;

  bool value = false;
  if (FieldManualStore::toggleFavorite(entries[entryIndex].path, &value)) {
    entries[entryIndex].favorite = value;
    rebuildVisible();
    requestUpdate();
  }
}

void FieldManualActivity::activateIndex(const int index) {
  if (index == 0) {
    openSearch();
    return;
  }
  if (index == 1) {
    cycleFilter();
    return;
  }
  if (index == 2) {
    rebuildIndex();
    return;
  }

  const int visibleIndex = index - CONTROL_ROWS;
  if (visibleIndex < 0 || visibleIndex >= static_cast<int>(visible.size())) return;
  const auto& entry = entries[visible[visibleIndex]];

  if (entry.checklist) {
    startActivityForResult(
        std::make_unique<ChecklistActivity>(renderer, mappedInput, entry.path),
        [this](const ActivityResult&) { requestUpdate(); });
    return;
  }

  app.clearTapFlash();
  activityManager.goToReader(entry.path);
}

void FieldManualActivity::onRowLongPress(const int index) {
  if (index < CONTROL_ROWS) return;
  toggleFavoriteAt(index - CONTROL_ROWS);
}

bool FieldManualActivity::handleButtons() {
  if (mappedInput.wasReleased(MappedInputManager::Button::Confirm)) {
    const int selected = nav.selected;
    if (selected >= CONTROL_ROWS && mappedInput.getHeldTime() >= ACTION_HOLD_MS) {
      toggleFavoriteAt(selected - CONTROL_ROWS);
    } else if (selected >= 0 && selected < listCount()) {
      activateIndex(selected);
    }
    return true;
  }

  if (mappedInput.wasReleased(MappedInputManager::Button::Back)) {
    if (!searchQuery.empty()) {
      searchQuery.clear();
      nav.selected = 0;
      rebuildVisible();
      requestUpdate();
    } else {
      finish();
    }
    return true;
  }

  return false;
}

void FieldManualActivity::buildScreen(UiScreen& screen) {
  const auto& metrics = UITheme::getInstance().getMetrics();
  screen.setContentMarginFromScreen(
      fui::Insets{static_cast<int16_t>(metrics.topPadding + metrics.headerHeight), 0,
                  static_cast<int16_t>(metrics.buttonHintsHeight), 0});
  screen.spacer(static_cast<int16_t>(metrics.verticalSpacing));

  fui::ListProps props;
  props.items = rowItems.data();
  props.count = static_cast<uint16_t>(rowItems.size());
  props.action = ACTION_ROW;
  props.inputMask = fui::InputTouch | fui::InputLongPress;
  fui::TextStyle label = screen.theme().smallText;
  label.bold = true;
  props.labelText = label;
  props.valueInset = 8;
  syncListViewport(screen, props, true);
  screen.list(props);
}

void FieldManualActivity::drawFooter() {
  const auto labels = mappedInput.mapLabels(
      searchQuery.empty() ? "Back" : "Wyczysc",
      nav.selected >= CONTROL_ROWS ? "Open / Hold: *" : "Select",
      tr(STR_DIR_UP), tr(STR_DIR_DOWN));
  GUI.drawButtonHints(renderer, labels.btn1, labels.btn2, labels.btn3, labels.btn4);
}
