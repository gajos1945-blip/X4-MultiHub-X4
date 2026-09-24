#include "GatewayDiagnosticsClient.h"

#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <WiFi.h>

#include "GatewayAuth.h"

namespace {
constexpr int CONNECT_TIMEOUT_MS = 5000;
constexpr int REQUEST_TIMEOUT_MS = 10000;

bool gatewayAllowed(const std::string& value) {
  return value.rfind("http://", 0) == 0 &&
         value.size() >= 10 &&
         value.size() <= 160 &&
         value.find(' ') == std::string::npos;
}
}  // namespace

std::string GatewayDiagnosticsClient::baseUrl(const std::string& gateway) {
  std::string base = gateway;
  while (!base.empty() && base.back() == '/') base.pop_back();
  return base;
}

GatewayDiagnosticsResult GatewayDiagnosticsClient::run(
    const std::string& gateway) const {
  GatewayDiagnosticsResult result;
  result.attempted = true;

  if (WiFi.status() != WL_CONNECTED) {
    result.error = "Wi-Fi nie jest polaczone";
    return result;
  }

  if (!gatewayAllowed(gateway)) {
    result.error = "Ustaw poprawny X4 Data Gateway (http://)";
    return result;
  }

  const std::string base = baseUrl(gateway);

  {
    HTTPClient http;
    http.setConnectTimeout(CONNECT_TIMEOUT_MS);
    http.setTimeout(REQUEST_TIMEOUT_MS);

    const std::string url = base + "/health";
    if (!http.begin(url.c_str())) {
      result.error = "Nie mozna otworzyc /health";
      return result;
    }

    result.healthStatus = http.GET();
    if (result.healthStatus != 200) {
      result.error = "Gateway /health HTTP " +
                     std::to_string(result.healthStatus);
      http.end();
      return result;
    }

    const String body = http.getString();
    http.end();

    JsonDocument doc;
    if (deserializeJson(doc, body)) {
      result.error = "Nieprawidlowy JSON /health";
      return result;
    }

    result.healthOk = doc["ok"] | false;
    result.authRequired = doc["auth_required"] | false;
    const char* version = doc["version"] | "";
    result.version = version ? version : "";

    if (!result.healthOk) {
      result.error = "Gateway /health nie potwierdzil OK";
      return result;
    }
  }

  {
    HTTPClient http;
    http.setConnectTimeout(CONNECT_TIMEOUT_MS);
    http.setTimeout(REQUEST_TIMEOUT_MS);

    const std::string url = base + "/v1/ping";
    if (!http.begin(url.c_str())) {
      result.error = "Nie mozna otworzyc /v1/ping";
      return result;
    }

    GatewayAuth::addHeaderIfConfigured(http);
    result.pingStatus = http.GET();

    if (result.pingStatus == 401) {
      result.error = "Gateway token nie pasuje";
      http.end();
      return result;
    }

    if (result.pingStatus != 200) {
      result.error = "Gateway /v1/ping HTTP " +
                     std::to_string(result.pingStatus);
      http.end();
      return result;
    }

    const String body = http.getString();
    http.end();

    JsonDocument doc;
    if (deserializeJson(doc, body)) {
      result.error = "Nieprawidlowy JSON /v1/ping";
      return result;
    }

    result.pingOk = doc["ok"] | false;
    if (!result.pingOk) {
      result.error = "Gateway /v1/ping nie potwierdzil OK";
      return result;
    }
  }

  result.error.clear();
  return result;
}
