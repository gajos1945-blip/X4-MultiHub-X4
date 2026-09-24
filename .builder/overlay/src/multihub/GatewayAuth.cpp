#include "GatewayAuth.h"

#include "GatewayAuthStore.h"

namespace GatewayAuth {

void addHeaderIfConfigured(HTTPClient& http) {
  std::string token;
  if (!GatewayAuthStore::loadToken(token)) return;

  http.addHeader("X-X4-Token", token.c_str());
}

}  // namespace GatewayAuth
