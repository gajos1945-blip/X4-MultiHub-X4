#pragma once

#include <cstddef>
#include <string>

namespace GatewayAuthStore {

constexpr const char* ROOT = "/.x4-multihub/gateway";
constexpr const char* CONFIG_PATH = "/.x4-multihub/gateway/auth.json";
constexpr size_t MIN_TOKEN_LEN = 8;
constexpr size_t MAX_TOKEN_LEN = 96;

bool validToken(const std::string& token);
bool loadToken(std::string& token);
bool saveToken(const std::string& token);
bool clearToken();
bool isConfigured();

}  // namespace GatewayAuthStore
