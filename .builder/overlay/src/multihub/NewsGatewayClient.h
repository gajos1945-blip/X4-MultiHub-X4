#pragma once

#include <string>
#include <vector>

#include "NewsStore.h"

struct NewsFeedResponse {
  bool ok = false;
  std::string error;
  std::string feedTitle;
  std::vector<NewsArticle> articles;
};

class NewsGatewayClient {
 public:
  NewsFeedResponse fetch(const std::string& gateway,
                         const std::string& feedUrl,
                         int limit = 15) const;

 private:
  static std::string encode(const std::string& value);
  static std::string baseUrl(const std::string& gateway);
};
