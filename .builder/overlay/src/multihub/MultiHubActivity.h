#pragma once

#include <array>
#include <string>

#include "activities/UiListActivity.h"

class MultiHubActivity final : public UiListActivity {
 public:
  static constexpr int ROWS = 7;

 private:
  std::array<freeink::ui::ListItem, ROWS> rows{};
  std::array<std::string, ROWS> values{};
  std::string header = "X4 MultiHub";

  void rebuildRows();

 protected:
  int listCount() const override { return ROWS; }
  void buildScreen(UiScreen& screen) override;
  void activateIndex(int index) override;
  const char* headerTitle() const override { return header.c_str(); }

 public:
  MultiHubActivity(GfxRenderer& renderer, MappedInputManager& mappedInput)
      : UiListActivity("MultiHub", renderer, mappedInput) {}

  void onEnter() override;
};
