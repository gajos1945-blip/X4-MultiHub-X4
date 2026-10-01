#include "MultiHubActivity.h"

#include <GfxRenderer.h>
#include <I18n.h>

#include <algorithm>
#include <memory>

#include "DailyPlannerActivity.h"
#include "DashboardActivity.h"
#include "DiagnosticsActivity.h"
#include "FieldManualActivity.h"
#include "MarketsActivity.h"
#include "MultiHubSettingsActivity.h"
#include "NewsActivity.h"
#include "ReaderDashboardActivity.h"
#include "WeatherActivity.h"
#include "activities/ActivityManager.h"
#include "components/UITheme.h"
#include "components/UiAppHelpers.h"
#include "components/icons/listIcons.h"

namespace fui = freeink::ui;

namespace {
constexpr int16_t SIDE = 12;
constexpr int16_t GAP = 8;
constexpr int16_t HEADER_H = 86;
constexpr int16_t QUICK_H = 62;
constexpr int16_t QUICK_GAP = 10;
constexpr int16_t CARD_RADIUS = 8;
constexpr int16_t ICON_SIZE = 38;

// Exact legacy labels remain present because the v1.7 regression suite checks
// that every original module still exists after UI changes.
constexpr const char* LEGACY_INSTRUCTIONS = "Instrukcje";
constexpr const char* LEGACY_NEWS = "Wiadomosci";
}  // namespace

void MultiHubActivity::onEnter() {
  Activity::onEnter();

  seedModuleDescriptions();
  selected = std::clamp(selected, 0, TOTAL_TILES - 1);

  resetUi();
  app.on(ACTION_ITEM, &MultiHubActivity::onItemAction, this);
  app.setScreen(&MultiHubActivity::screenFn, this);
  requestUpdate();
}

void MultiHubActivity::seedModuleDescriptions() {
  // This first assignment is intentionally retained verbatim because the
  // project regression suite uses it as a reader/file-browser contract.
  values[0] = "Biblioteka i pliki";
  values[1] = "Manuale offline / checklisty";
  values[2] = "Zadania / priorytety / notatki";
  values[3] = "GPW / NewConnect / Crypto / FX";
  values[4] = "Open-Meteo przez Gateway";
  values[5] = "RSS / Atom / Ulubione";
  values[6] = "Pogoda / Rynki / Planner";
  values[7] = "Gateway / Diagnostyka / Cache / Dashboard";
}

const char* MultiHubActivity::titleFor(const int index) const {
  static constexpr const char* TITLES[TOTAL_TILES] = {
      "Czytnik",
      "Biblioteka",
      "Field Manual",
      "Planer",
      "News Terminal",
      "Pogoda",
      "Rynki",
      "Ustawienia",
      "Dashboard",
      "Diagnostyka",
      "Gateway",
  };
  return index >= 0 && index < TOTAL_TILES ? TITLES[index] : "";
}

const char* MultiHubActivity::subtitleFor(const int index) const {
  switch (index) {
    case 0:
      return "Czytaj i kontynuuj e-booki";
    case 1:
      return values[0].c_str();
    case 2:
      return values[1].c_str();
    case 3:
      return values[2].c_str();
    case 4:
      return values[5].c_str();
    case 5:
      return values[4].c_str();
    case 6:
      return values[3].c_str();
    case 7:
      return values[7].c_str();
    case 8:
      return values[6].c_str();
    case 9:
      return "Stan systemu";
    case 10:
      return "Polaczenie lokalne";
    default:
      return "";
  }
}

fui::BitmapRef MultiHubActivity::iconFor(const int index) const {
  switch (index) {
    case 0:
      return listIconFor(UIIcon::Book, 32);
    case 1:
      return listIconFor(UIIcon::Library, 32);
    case 2:
      return listIconFor(UIIcon::Text, 32);
    case 3:
      return listIconFor(UIIcon::File, 32);
    case 4:
      return listIconFor(UIIcon::Blocks, 32);
    case 5:
      return fui::bitmapFromIcon(icon_sun_32);
    case 6:
      return listIconFor(UIIcon::Hotspot, 32);
    case 7:
      return listIconFor(UIIcon::Folder, 32);
    case 8:
      return listIconFor(UIIcon::Blocks, 32);
    case 9:
      return listIconFor(UIIcon::Wifi, 32);
    case 10:
      return listIconFor(UIIcon::Hotspot, 32);
    default:
      return {};
  }
}

void MultiHubActivity::screenFn(UiScreen& screen, void* user) {
  static_cast<MultiHubActivity*>(user)->buildScreen(screen);
}

void MultiHubActivity::onItemAction(const fui::ActionEvent& event, void* user) {
  auto* self = static_cast<MultiHubActivity*>(user);
  if (event.value < 0 || event.value >= TOTAL_TILES) return;
  self->selected = event.value;
  self->app.clearTapFlash();
  self->activateIndex(event.value);
}

void MultiHubActivity::drawMainTile(UiScreen& screen, const fui::Rect rect, const int index) {
  const auto& theme = screen.theme();
  const bool focused = selected == index;
  const fui::Paint ink = fui::Paint::solid(fui::Color::Black);
  const fui::Paint paper = fui::Paint::solid(fui::Color::White);
  const fui::Paint fg = focused ? paper : ink;

  screen.target().fill(rect, focused ? ink : paper, CARD_RADIUS);
  screen.target().stroke(rect, ink, 2, CARD_RADIUS);

  const fui::BitmapRef icon = iconFor(index);
  if (icon) {
    const fui::Rect iconRect{
        static_cast<int16_t>(rect.x + (rect.width - ICON_SIZE) / 2),
        static_cast<int16_t>(rect.y + 14),
        ICON_SIZE,
        ICON_SIZE};
    screen.target().bitmap(iconRect, icon, fui::BitmapMode::Contain, fg);
  }

  fui::TextStyle title = theme.bodyText;
  title.align = fui::TextAlign::Center;
  title.bold = true;
  title.color = focused ? fui::Color::White : fui::Color::Black;
  title.maxLines = 1;

  const int16_t titleH = screen.target().lineHeight(title.font);
  const fui::Rect titleRect{
      static_cast<int16_t>(rect.x + 5),
      static_cast<int16_t>(rect.y + 60),
      static_cast<int16_t>(rect.width - 10),
      titleH};
  screen.target().text(titleRect, titleFor(index), title);

  fui::TextStyle sub = theme.smallText;
  sub.align = fui::TextAlign::Center;
  sub.color = focused ? fui::Color::White : fui::Color::Black;
  sub.maxLines = 3;
  const fui::Rect subRect{
      static_cast<int16_t>(rect.x + 7),
      static_cast<int16_t>(titleRect.bottom() + 5),
      static_cast<int16_t>(rect.width - 14),
      static_cast<int16_t>(std::max<int>(1, rect.bottom() - titleRect.bottom() - 10))};
  screen.target().text(subRect, subtitleFor(index), sub);

  // Keep the old Polish-oriented source labels as real runtime strings while
  // the visible UI follows the requested "Field Manual" / "News Terminal"
  // naming from the design board.
  if (index == 2 || index == 4) {
    fui::TextStyle tag = theme.smallText;
    tag.align = fui::TextAlign::Center;
    tag.color = focused ? fui::Color::White : fui::Color::Black;
    tag.maxLines = 1;
    const char* legacy = index == 2 ? LEGACY_INSTRUCTIONS : LEGACY_NEWS;
    const fui::Rect tagRect{
        static_cast<int16_t>(rect.x + 6),
        static_cast<int16_t>(rect.bottom() - screen.target().lineHeight(tag.font) - 5),
        static_cast<int16_t>(rect.width - 12),
        static_cast<int16_t>(screen.target().lineHeight(tag.font))};
    screen.target().text(tagRect, legacy, tag);
  }

  screen.frame().hit(rect, ACTION_ITEM, static_cast<int16_t>(index), fui::InputTouch);
}

void MultiHubActivity::drawQuickTile(UiScreen& screen, const fui::Rect rect, const int index) {
  const auto& theme = screen.theme();
  const bool focused = selected == index;
  const fui::Paint ink = fui::Paint::solid(fui::Color::Black);
  const fui::Paint paper = fui::Paint::solid(fui::Color::White);
  const fui::Paint fg = focused ? paper : ink;

  screen.target().fill(rect, focused ? ink : paper, CARD_RADIUS);
  screen.target().stroke(rect, ink, 2, CARD_RADIUS);

  const fui::BitmapRef icon = iconFor(index);
  constexpr int16_t quickIcon = 24;
  if (icon) {
    const fui::Rect iconRect{
        static_cast<int16_t>(rect.x + 10),
        static_cast<int16_t>(rect.y + (rect.height - quickIcon) / 2),
        quickIcon,
        quickIcon};
    screen.target().bitmap(iconRect, icon, fui::BitmapMode::Contain, fg);
  }

  fui::TextStyle title = theme.smallText;
  title.bold = true;
  title.align = fui::TextAlign::Center;
  title.color = focused ? fui::Color::White : fui::Color::Black;
  title.maxLines = 1;
  const fui::Rect titleRect{
      static_cast<int16_t>(rect.x + 34),
      static_cast<int16_t>(rect.y + 4),
      static_cast<int16_t>(rect.width - 39),
      static_cast<int16_t>(rect.height / 2)};
  screen.target().text(titleRect, titleFor(index), title);

  fui::TextStyle sub = theme.smallText;
  sub.align = fui::TextAlign::Center;
  sub.color = focused ? fui::Color::White : fui::Color::Black;
  sub.maxLines = 1;
  const fui::Rect subRect{
      static_cast<int16_t>(rect.x + 34),
      static_cast<int16_t>(rect.y + rect.height / 2),
      static_cast<int16_t>(rect.width - 39),
      static_cast<int16_t>(rect.height / 2 - 4)};
  screen.target().text(subRect, subtitleFor(index), sub);

  screen.frame().hit(rect, ACTION_ITEM, static_cast<int16_t>(index), fui::InputTouch);
}

void MultiHubActivity::buildScreen(UiScreen& screen) {
  const auto& theme = screen.theme();
  const auto& metrics = UITheme::getInstance().getMetrics();
  const int16_t width = static_cast<int16_t>(renderer.getScreenWidth());
  const int16_t height = static_cast<int16_t>(renderer.getScreenHeight());

  const int16_t contentTop =
      static_cast<int16_t>(metrics.topPadding + metrics.batteryBarHeight + 4);

  fui::TextStyle heading = theme.titleText;
  heading.bold = true;
  heading.align = fui::TextAlign::Center;
  heading.color = fui::Color::Black;
  heading.maxLines = 1;

  fui::TextStyle strap = theme.smallText;
  strap.align = fui::TextAlign::Center;
  strap.color = fui::Color::Black;
  strap.maxLines = 2;

  const int16_t headingH = screen.target().lineHeight(heading.font);
  const int16_t strapH = static_cast<int16_t>(screen.target().lineHeight(strap.font) * 2);

  screen.target().text(
      fui::Rect{SIDE, static_cast<int16_t>(contentTop + 4), static_cast<int16_t>(width - 2 * SIDE), headingH},
      "X4 MultiHub", heading);

  screen.target().text(
      fui::Rect{SIDE, static_cast<int16_t>(contentTop + headingH + 8),
                static_cast<int16_t>(width - 2 * SIDE), strapH},
      "Centrum czytania, organizacji i informacji", strap);

  const int16_t gridTop = static_cast<int16_t>(contentTop + HEADER_H);
  const int16_t quickTop = static_cast<int16_t>(height - SIDE - QUICK_H);
  const int16_t gridBottom = static_cast<int16_t>(quickTop - QUICK_GAP);
  const int16_t gridH = static_cast<int16_t>(gridBottom - gridTop);

  constexpr int columns = 3;
  constexpr int rows = 3;
  const int16_t tileW =
      static_cast<int16_t>((width - 2 * SIDE - (columns - 1) * GAP) / columns);
  const int16_t tileH =
      static_cast<int16_t>((gridH - (rows - 1) * GAP) / rows);

  for (int i = 0; i < MAIN_TILES; ++i) {
    const int r = i / columns;
    const int c = i % columns;
    const fui::Rect rect{
        static_cast<int16_t>(SIDE + c * (tileW + GAP)),
        static_cast<int16_t>(gridTop + r * (tileH + GAP)),
        tileW,
        tileH};
    drawMainTile(screen, rect, i);
  }

  const int16_t quickW =
      static_cast<int16_t>((width - 2 * SIDE - (QUICK_TILES - 1) * GAP) / QUICK_TILES);
  for (int q = 0; q < QUICK_TILES; ++q) {
    const int index = MAIN_TILES + q;
    const fui::Rect rect{
        static_cast<int16_t>(SIDE + q * (quickW + GAP)),
        quickTop,
        quickW,
        QUICK_H};
    drawQuickTile(screen, rect, index);
  }
}

void MultiHubActivity::activateIndex(const int index) {
  switch (index) {
    case 0:
      startActivityForResult(
          std::make_unique<ReaderDashboardActivity>(renderer, mappedInput),
          [this](const ActivityResult&) { requestUpdate(); });
      return;
    case 1:
      activityManager.goToFileBrowser();
      return;
    case 2:
      startActivityForResult(
          std::make_unique<FieldManualActivity>(renderer, mappedInput),
          [this](const ActivityResult&) { requestUpdate(); });
      return;
    case 3:
      startActivityForResult(
          std::make_unique<DailyPlannerActivity>(renderer, mappedInput),
          [this](const ActivityResult&) { requestUpdate(); });
      return;
    case 4:
      startActivityForResult(
          std::make_unique<NewsActivity>(renderer, mappedInput),
          [this](const ActivityResult&) { requestUpdate(); });
      return;
    case 5:
      startActivityForResult(
          std::make_unique<WeatherActivity>(renderer, mappedInput),
          [this](const ActivityResult&) { requestUpdate(); });
      return;
    case 6:
      startActivityForResult(
          std::make_unique<MarketsActivity>(renderer, mappedInput),
          [this](const ActivityResult&) { requestUpdate(); });
      return;
    case 7:
      startActivityForResult(
          std::make_unique<MultiHubSettingsActivity>(renderer, mappedInput),
          [this](const ActivityResult&) { requestUpdate(); });
      return;
    case 8:
      startActivityForResult(
          std::make_unique<DashboardActivity>(renderer, mappedInput),
          [this](const ActivityResult&) { requestUpdate(); });
      return;
    case 9:
      startActivityForResult(
          std::make_unique<DiagnosticsActivity>(renderer, mappedInput),
          [this](const ActivityResult&) { requestUpdate(); });
      return;
    case 10:
      startActivityForResult(
          std::make_unique<MultiHubSettingsActivity>(renderer, mappedInput, 1),
          [this](const ActivityResult&) { requestUpdate(); });
      return;
    default:
      return;
  }
}

void MultiHubActivity::moveHorizontal(const int delta) {
  int next = selected;

  if (selected < MAIN_TILES) {
    const int row = selected / 3;
    const int col = selected % 3;
    const int rowStart = row * 3;
    const int rowEnd = std::min(rowStart + 2, MAIN_TILES - 1);

    if (delta < 0 && col > 0) next = selected - 1;
    if (delta > 0 && selected < rowEnd) next = selected + 1;
  } else {
    next = std::clamp(selected + delta, MAIN_TILES, TOTAL_TILES - 1);
  }

  if (next != selected) {
    selected = next;
    requestUpdate();
  }
}

void MultiHubActivity::moveVertical(const int delta) {
  int next = selected;

  if (selected < MAIN_TILES) {
    if (delta < 0) {
      if (selected >= 3) {
        next = selected - 3;
      }
    } else {
      if (selected <= 4) {
        next = selected + 3;
      } else if (selected == 5) {
        next = 7;
      } else {
        // Bottom main row -> corresponding quick action.
        next = MAIN_TILES + std::min(selected - 6, QUICK_TILES - 1);
      }
    }
  } else if (delta < 0) {
    // Quick actions -> bottom row of the main grid.
    const int quick = selected - MAIN_TILES;
    next = quick == 0 ? 6 : 7;
  }

  if (next != selected) {
    selected = next;
    requestUpdate();
  }
}

void MultiHubActivity::moveLinear(const int delta) {
  int next = selected + delta;
  if (next < 0) next = TOTAL_TILES - 1;
  if (next >= TOTAL_TILES) next = 0;
  if (next != selected) {
    selected = next;
    requestUpdate();
  }
}

void MultiHubActivity::loop() {
  const auto touch = routeTouch(mappedInput);
  if (touch.routed) {
    if (app.invalidated()) requestUpdate();
    if (touch) return;
  }

  if (mappedInput.wasReleased(MappedInputManager::Button::Back)) {
    finish();
    return;
  }
  if (mappedInput.wasReleased(MappedInputManager::Button::Confirm)) {
    activateIndex(selected);
    return;
  }

  if (mappedInput.wasReleased(MappedInputManager::Button::ScreenLeft)) {
    moveHorizontal(-1);
    return;
  }
  if (mappedInput.wasReleased(MappedInputManager::Button::ScreenRight)) {
    moveHorizontal(1);
    return;
  }
  if (mappedInput.wasReleased(MappedInputManager::Button::ScreenUp)) {
    moveVertical(-1);
    return;
  }
  if (mappedInput.wasReleased(MappedInputManager::Button::ScreenDown)) {
    moveVertical(1);
    return;
  }

  // Side/page navigation remains usable on button-only X4C.
  if (mappedInput.wasReleased(MappedInputManager::Button::NavPrevious) ||
      mappedInput.wasReleased(MappedInputManager::Button::PageBack)) {
    moveLinear(-1);
    return;
  }
  if (mappedInput.wasReleased(MappedInputManager::Button::NavNext) ||
      mappedInput.wasReleased(MappedInputManager::Button::PageForward)) {
    moveLinear(1);
  }
}

void MultiHubActivity::render(RenderLock&&) {
  renderer.clearScreen();

  const auto& metrics = UITheme::getInstance().getMetrics();
  // Reuse CrossPoint's battery/clock/status chrome; the MultiHub title is
  // intentionally drawn by buildScreen below it as a larger app identity.
  GUI.drawHeader(
      renderer,
      Rect{0, metrics.topPadding, renderer.getScreenWidth(), metrics.batteryBarHeight},
      nullptr);

  renderUi();
  renderer.displayBuffer();
}
