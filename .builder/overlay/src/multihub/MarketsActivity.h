#pragma once

#include <string>
#include <vector>

#include "MarketGatewayClient.h"
#include "MarketStore.h"
#include "activities/UiListActivity.h"

class MarketsActivity final : public UiListActivity {
 private:
  static constexpr int CONTROL_ROWS = 4;

  std::string gateway;
  std::string asset = "pl";
  std::string lastError;
  std::string header = "Markets & Weather";

  std::vector<MarketFavorite> favorites;
  std::vector<MarketQuote> quotes;
  std::vector<std::string> rowLabels;
  std::vector<std::string> rowValues;
  std::vector<std::string> rowSubtitles;
  std::vector<freeink::ui::ListItem> rows;

  MarketGatewayClient client;

  void reload();
  void rebuildRows();
  void editGateway();
  void cycleAsset();
  void searchAndAdd();
  void refreshQuotes();
  void removeFavoriteAt(int favoriteIndex);

  const MarketQuote* quoteFor(const std::string& symbol) const;
  const char* assetName() const;
  static std::string priceText(const MarketQuote* quote);

 protected:
  int listCount() const override { return static_cast<int>(rows.size()); }
  void buildScreen(UiScreen& screen) override;
  void activateIndex(int index) override;
  void onRowLongPress(int index) override;
  const char* headerTitle() const override { return header.c_str(); }
  void drawFooter() override;

 public:
  MarketsActivity(GfxRenderer& renderer, MappedInputManager& mappedInput)
      : UiListActivity("Markets", renderer, mappedInput, true) {}

  void onEnter() override;
  void onExit() override;
};
