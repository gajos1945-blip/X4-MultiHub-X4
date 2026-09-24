#pragma once

#include <cstddef>
#include <string>
#include <vector>

struct NewsFeed {
  std::string name;
  std::string url;
};

struct NewsArticle {
  std::string title;
  std::string link;
  std::string source;
  std::string published;
  std::string summary;
};

namespace NewsStore {

constexpr size_t MAX_FEEDS = 30;
constexpr size_t MAX_FAVORITES = 100;

constexpr const char* ROOT = "/.x4-multihub/news";
constexpr const char* FEEDS_PATH = "/.x4-multihub/news/feeds.jsonl";
constexpr const char* FAVORITES_PATH = "/.x4-multihub/news/favorites.jsonl";
constexpr const char* ARTICLE_PATH = "/.x4-multihub/news/current_article.txt";

bool loadFeeds(std::vector<NewsFeed>& feeds);
bool saveFeeds(const std::vector<NewsFeed>& feeds);
bool addFeed(const NewsFeed& feed);
bool removeFeed(const std::string& url);

bool loadFavorites(std::vector<NewsArticle>& items);
bool saveFavorites(const std::vector<NewsArticle>& items);
bool toggleFavorite(const NewsArticle& article, bool* nowFavorite = nullptr);
bool isFavorite(const std::string& link);

bool writeArticleText(const NewsArticle& article);

bool validFeedName(const std::string& name);
bool validFeedUrl(const std::string& url);

}  // namespace NewsStore
