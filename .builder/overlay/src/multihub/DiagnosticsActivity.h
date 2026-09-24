#pragma once

#include <array>
#include <string>

#include "GatewayDiagnosticsClient.h"
#include "activities/UiListActivity.h"

class DiagnosticsActivity final : public UiListActivity {
 public:
  static constexpr int ROWS = 7;

 private:
  std::array<std::string, ROWS> values{};
  std::array<freeink::ui::ListItem, ROWS> rows{};

  std::string header = "Diagnostyka";
  std::string gateway;
  std::string localTime;
  GatewayDiagnosticsResult gatewayResult;
  bool storageTestRun = false;
  bool storageOk = false;

  void reloadLocal();
  void rebuildRows();
  void runGatewayTest();
  void runStorageTest();
  void exportReport();
  std::string buildReport() const;

 protected:
  int listCount() const override { return ROWS; }
  void buildScreen(UiScreen& screen) override;
  void activateIndex(int index) override;
  const char* headerTitle() const override { return header.c_str(); }

 public:
  DiagnosticsActivity(GfxRenderer& renderer, MappedInputManager& mappedInput)
      : UiListActivity("Diagnostics", renderer, mappedInput) {}

  void onEnter() override;
};
