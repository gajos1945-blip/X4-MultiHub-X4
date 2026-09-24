#pragma once

#include <HTTPClient.h>

namespace GatewayAuth {

// Adds the X4 gateway token header if one is configured on microSD.
// No header is added when auth is not configured.
void addHeaderIfConfigured(HTTPClient& http);

}  // namespace GatewayAuth
