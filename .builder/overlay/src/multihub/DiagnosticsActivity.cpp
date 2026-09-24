#include "DiagnosticsActivity.h"

#include <GfxRenderer.h>
#include <WiFi.h>

#include <string>

#include "DiagnosticsStore.h"
#include "GatewayAuthStore.h"
#include "MarketStore.h"
#include "TimeService.h"
#include "components/UITheme.h"

namespace fui = freeink::ui;

namespace {
const char* LABELS[DiagnosticsActivity::ROWS] = {
    "Wi-Fi",
    "X4 Data Gateway",
    "Gateway /health",
    "Gateway auth /v1/ping",
    "microSD write test",
    "Czas lokalny",
    "Eksport raportu",
};

std::string yesNo(const bool value) {
  return value ? "YES" : "NO";
}
}  // namespace

void DiagnosticsActivity::onEnter() {
  UiListActivity::onEnter();
  reloadLocal();
}

void DiagnosticsActivity::reloadLocal() {
  MarketStore::loadGateway(gateway);
  TimeService::localNow(localTime);
  rebuildRows();
}

void DiagnosticsActivity::rebuildRows() {
  if (WiFi.status() == WL_CONNECTED) {
    values[0] = "POLACZONO | RSSI " + std::to_string(WiFi.RSSI()) + " dBm";
  } else {
    values[0] = "ROZLACZONO";
  }

  values[1] = gateway.empty() ? "BRAK" : gateway;

  if (!gatewayResult.attempted) {
    values[2] = "NIE TESTOWANO";
    values[3] = GatewayAuthStore::isConfigured()
                    ? "TOKEN USTAWIONY | NIE TESTOWANO"
                    : "TOKEN BRAK | NIE TESTOWANO";
  } else {
    if (gatewayResult.healthOk) {
      values[2] = "OK";
      if (!gatewayResult.version.empty()) {
        values[2] += " | v";
        values[2] += gatewayResult.version;
      }
    } else {
      values[2] = gatewayResult.healthStatus > 0
                      ? "HTTP " + std::to_string(gatewayResult.healthStatus)
                      : "ERROR";
    }

    if (gatewayResult.pingOk) {
      values[3] = gatewayResult.authRequired
                      ? "OK | AUTH REQUIRED"
                      : "OK | AUTH DISABLED";
    } else if (gatewayResult.pingStatus == 401) {
      values[3] = "401 | TOKEN NIE PASUJE";
    } else {
      values[3] = gatewayResult.error.empty()
                      ? "ERROR"
                      : gatewayResult.error;
    }
  }

  values[4] = !storageTestRun
                  ? "NIE TESTOWANO"
                  : (storageOk ? "OK" : "ERROR");

  values[5] = localTime.empty() ? "UNKNOWN" : localTime;
  values[6] = DiagnosticsStore::REPORT_PATH;

  for (int i = 0; i < ROWS; ++i) {
    rows[i] = {};
    rows[i].label = LABELS[i];
    rows[i].value = values[i].c_str();
    rows[i].actionValue = static_cast<int16_t>(i);
  }
}

void DiagnosticsActivity::runGatewayTest() {
  MarketStore::loadGateway(gateway);

  GatewayDiagnosticsClient client;
  gatewayResult = client.run(gateway);

  if (gatewayResult.healthOk && gatewayResult.pingOk) {
    header = "Gateway OK";
  } else {
    header = gatewayResult.error.empty()
                 ? "Gateway ERROR"
                 : gatewayResult.error;
  }

  rebuildRows();
  requestUpdate();
}

void DiagnosticsActivity::runStorageTest() {
  storageTestRun = true;
  storageOk = DiagnosticsStore::storageWriteTest();
  header = storageOk ? "microSD OK" : "microSD ERROR";

  rebuildRows();
  requestUpdate();
}

std::string DiagnosticsActivity::buildReport() const {
  std::string report;
  report.reserve(2048);

  report += "X4 MultiHub diagnostics\n";
  report += "version=1.7-dev\n";
  report += "physical_x4_verified=NO\n";
  report += "image_type=APPLICATION BIN\n";
  report += "automatic_erase=NO\n";

  report += "wifi_connected=";
  report += WiFi.status() == WL_CONNECTED ? "YES\n" : "NO\n";
  if (WiFi.status() == WL_CONNECTED) {
    report += "wifi_rssi_dbm=" + std::to_string(WiFi.RSSI()) + "\n";
  }

  report += "gateway_url=";
  report += gateway.empty() ? "UNKNOWN\n" : gateway + "\n";

  report += "gateway_token_configured=";
  report += yesNo(GatewayAuthStore::isConfigured()) + "\n";

  report += "gateway_test_attempted=" + yesNo(gatewayResult.attempted) + "\n";
  report += "gateway_health_ok=" + yesNo(gatewayResult.healthOk) + "\n";
  report += "gateway_health_http=" +
            std::to_string(gatewayResult.healthStatus) + "\n";
  report += "gateway_version=" +
            (gatewayResult.version.empty() ? "UNKNOWN" : gatewayResult.version) +
            "\n";
  report += "gateway_auth_required=" +
            yesNo(gatewayResult.authRequired) + "\n";
  report += "gateway_ping_ok=" + yesNo(gatewayResult.pingOk) + "\n";
  report += "gateway_ping_http=" +
            std::to_string(gatewayResult.pingStatus) + "\n";

  if (!gatewayResult.error.empty()) {
    report += "gateway_error=" + gatewayResult.error + "\n";
  }

  report += "storage_test_run=" + yesNo(storageTestRun) + "\n";
  report += "storage_write_ok=" + yesNo(storageOk) + "\n";
  report += "local_time=" +
            (localTime.empty() ? std::string("UNKNOWN") : localTime) + "\n";

  // IMPORTANT: the gateway access token value is intentionally never added.
  report += "gateway_token_value=REDACTED\n";

  return report;
}

void DiagnosticsActivity::exportReport() {
  TimeService::localNow(localTime);

  const std::string report = buildReport();
  if (DiagnosticsStore::writeReport(report)) {
    header = "Raport zapisany";
  } else {
    header = "Blad zapisu raportu";
  }

  rebuildRows();
  requestUpdate();
}

void DiagnosticsActivity::activateIndex(const int index) {
  switch (index) {
    case 0:
      reloadLocal();
      requestUpdate();
      return;
    case 1:
    case 2:
    case 3:
      runGatewayTest();
      return;
    case 4:
      runStorageTest();
      return;
    case 5:
      TimeService::localNow(localTime);
      rebuildRows();
      requestUpdate();
      return;
    case 6:
      exportReport();
      return;
    default:
      return;
  }
}

void DiagnosticsActivity::buildScreen(UiScreen& screen) {
  const auto& metrics = UITheme::getInstance().getMetrics();
  screen.setContentMarginFromScreen(
      fui::Insets{static_cast<int16_t>(metrics.topPadding + metrics.headerHeight), 0,
                  static_cast<int16_t>(metrics.buttonHintsHeight), 0});
  screen.spacer(static_cast<int16_t>(metrics.verticalSpacing));

  fui::ListProps props;
  props.items = rows.data();
  props.count = ROWS;
  props.action = ACTION_ROW;
  props.inputMask = fui::InputTouch;
  props.valueInset = 8;
  props.labelText = screen.theme().smallText;
  props.labelText.maxLines = 3;
  syncListViewport(screen, props);
  screen.list(props);
}
