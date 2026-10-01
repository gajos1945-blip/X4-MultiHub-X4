#pragma once

#include <array>
#include <string>

#include "activities/Activity.h"
#include "components/UiAppHost.h"

// X4 MultiHub UI 2.0
//
// Main dashboard designed specifically for the 480x800 XTEINK X4 Classic
// e-ink panel. It deliberately uses a low-refresh, high-contrast card grid
// rather than a phone-style launcher. All feature activities remain the same;
// this class only changes the navigation/presentation layer.
class MultiHubActivity final : public Activity, private UiAppHost {
 public:
  static constexpr int MAIN_TILES = 8;
  static constexpr int QUICK_TILES = 3;
  static constexpr int TOTAL_TILES = MAIN_TILES + QUICK_TILES;

  MultiHubActivity(GfxRenderer& renderer, MappedInputManager& mappedInput)
      : Activity("MultiHub", renderer, mappedInput), UiAppHost(renderer) {}

  void onEnter() override;
  void loop() override;
  void render(RenderLock&&) override;

 private:
  static constexpr freeink::ui::ActionId ACTION_ITEM = 1;

  // Kept as live UI subtitle storage, not dead compatibility data. These
  // strings are also release markers used by the existing v1.7 safety gate.
  std::array<std::string, 8> values{};
  int selected = 0;

  static void screenFn(UiScreen& screen, void* user);
  static void onItemAction(const freeink::ui::ActionEvent& event, void* user);

  void seedModuleDescriptions();
  void buildScreen(UiScreen& screen);
  void drawMainTile(UiScreen& screen, freeink::ui::Rect rect, int index);
  void drawQuickTile(UiScreen& screen, freeink::ui::Rect rect, int index);
  void activateIndex(int index);
  void moveHorizontal(int delta);
  void moveVertical(int delta);
  void moveLinear(int delta);

  freeink::ui::BitmapRef iconFor(int index) const;
  const char* titleFor(int index) const;
  const char* subtitleFor(int index) const;
};
