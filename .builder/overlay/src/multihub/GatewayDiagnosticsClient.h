#pragma once

#include <string>

struct GatewayDiagnosticsResult {
  bool attempted = false;
  bool healthOk = false;
  bool pingOk = false;
  bool authRequired = false;

  int healthStatus = 0;
  int pingStatus = 0;

  std::string version;
  std::string error;
};

class GatewayDiagnosticsClient {
 public:
  GatewayDiagnosticsResult run(const std::string& gateway) const;

 private:
  static std::string baseUrl(const std::string& gateway);
};
