#pragma once

#include <string>
#include <vector>

#include "MarketGatewayClient.h"
#include "activities/UiListActivity.h"

class MarketSearchResultsActivity final : public UiListActivity {
 private:
  std::vector<MarketSearchItem> items;
  std::vector<std::string> values;
  std::vector<freeink::ui::ListItem> rows;
  std::string header = "Wyniki";

  void rebuildRows();

 protected:
  int listCount() const override { return static_cast<int>(rows.size()); }
  void buildScreen(UiScreen& screen) override;
  void activateIndex(int index) override;
  const char* headerTitle() const override { return header.c_str(); }

 public:
  MarketSearchResultsActivity(GfxRenderer& renderer,
                              MappedInputManager& mappedInput,
                              std::vector<MarketSearchItem> items)
      : UiListActivity("MarketSearchResults", renderer, mappedInput),
        items(std::move(items)) {}

  void onEnter() override;
};
