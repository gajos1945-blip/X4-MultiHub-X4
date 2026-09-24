#include "NewsStore.h"

#include <ArduinoJson.h>
#include <HalStorage.h>

#include <algorithm>
#include <string>
#include <utility>

namespace {
constexpr const char* MOD = "NewsStore";
constexpr size_t MAX_LINE_BYTES = 4096;

bool ensureRoot() {
  if (!Storage.ensureDirectoryExists("/.x4-multihub")) return false;
  return Storage.ensureDirectoryExists(NewsStore::ROOT);
}

bool readLine(HalFile& file, std::string& line) {
  line.clear();
  while (file.available()) {
    const int ch = file.read();
    if (ch < 0) break;
    if (ch == '\n') return true;
    if (ch == '\r') continue;

    if (line.size() >= MAX_LINE_BYTES) {
      while (file.available()) {
        const int c = file.read();
        if (c < 0 || c == '\n') break;
      }
      line.clear();
      return true;
    }
    line.push_back(static_cast<char>(ch));
  }
  return !line.empty();
}

bool writeJsonLine(HalFile& file, JsonDocument& doc) {
  String raw;
  serializeJson(doc, raw);
  if (file.write(reinterpret_cast<const uint8_t*>(raw.c_str()), raw.length()) != raw.length()) {
    return false;
  }
  return file.write(static_cast<uint8_t>('\n')) == 1;
}
}  // namespace

namespace NewsStore {

bool validFeedName(const std::string& name) {
  return !name.empty() && name.size() <= 80 &&
         name.find('\n') == std::string::npos &&
         name.find('\r') == std::string::npos;
}

bool validFeedUrl(const std::string& url) {
  if (url.size() < 10 || url.size() > 500) return false;
  if (url.find(' ') != std::string::npos) return false;
  return url.rfind("https://", 0) == 0 || url.rfind("http://", 0) == 0;
}

bool loadFeeds(std::vector<NewsFeed>& feeds) {
  feeds.clear();

  HalFile file;
  if (!Storage.openFileForRead(MOD, FEEDS_PATH, file)) return false;

  std::string line;
  while (feeds.size() < MAX_FEEDS && readLine(file, line)) {
    if (line.empty()) continue;
    JsonDocument doc;
    if (deserializeJson(doc, line)) continue;

    const char* name = doc["name"] | nullptr;
    const char* url = doc["url"] | nullptr;
    if (!name || !url || !validFeedName(name) || !validFeedUrl(url)) continue;

    feeds.push_back(NewsFeed{std::string(name), std::string(url)});
  }
  file.close();
  return true;
}

bool saveFeeds(const std::vector<NewsFeed>& feeds) {
  if (!ensureRoot()) return false;

  const std::string temp = std::string(FEEDS_PATH) + ".tmp";
  HalFile file;
  if (!Storage.openFileForWrite(MOD, temp, file)) return false;

  const size_t count = std::min(feeds.size(), MAX_FEEDS);
  for (size_t i = 0; i < count; ++i) {
    if (!validFeedName(feeds[i].name) || !validFeedUrl(feeds[i].url)) continue;

    JsonDocument doc;
    doc["name"] = feeds[i].name;
    doc["url"] = feeds[i].url;
    if (!writeJsonLine(file, doc)) {
      file.close();
      Storage.remove(temp.c_str());
      return false;
    }
  }

  file.flush();
  file.close();
  Storage.remove(FEEDS_PATH);
  if (!Storage.rename(temp.c_str(), FEEDS_PATH)) {
    Storage.remove(temp.c_str());
    return false;
  }
  return true;
}

bool addFeed(const NewsFeed& feed) {
  if (!validFeedName(feed.name) || !validFeedUrl(feed.url)) return false;

  std::vector<NewsFeed> feeds;
  loadFeeds(feeds);

  const auto it = std::find_if(
      feeds.begin(), feeds.end(),
      [&](const NewsFeed& item) { return item.url == feed.url; });
  if (it != feeds.end()) return true;
  if (feeds.size() >= MAX_FEEDS) return false;

  feeds.push_back(feed);
  return saveFeeds(feeds);
}

bool removeFeed(const std::string& url) {
  std::vector<NewsFeed> feeds;
  if (!loadFeeds(feeds)) return false;

  const size_t before = feeds.size();
  feeds.erase(
      std::remove_if(feeds.begin(), feeds.end(),
                     [&](const NewsFeed& item) { return item.url == url; }),
      feeds.end());
  if (feeds.size() == before) return false;
  return saveFeeds(feeds);
}

bool loadFavorites(std::vector<NewsArticle>& items) {
  items.clear();

  HalFile file;
  if (!Storage.openFileForRead(MOD, FAVORITES_PATH, file)) return false;

  std::string line;
  while (items.size() < MAX_FAVORITES && readLine(file, line)) {
    if (line.empty()) continue;
    JsonDocument doc;
    if (deserializeJson(doc, line)) continue;

    const char* title = doc["title"] | nullptr;
    const char* link = doc["link"] | nullptr;
    if (!title || !*title || !link || !*link) continue;

    NewsArticle item;
    item.title = title;
    item.link = link;
    item.source = doc["source"] | "";
    item.published = doc["published"] | "";
    item.summary = doc["summary"] | "";
    items.push_back(std::move(item));
  }
  file.close();
  return true;
}

bool saveFavorites(const std::vector<NewsArticle>& items) {
  if (!ensureRoot()) return false;

  const std::string temp = std::string(FAVORITES_PATH) + ".tmp";
  HalFile file;
  if (!Storage.openFileForWrite(MOD, temp, file)) return false;

  const size_t count = std::min(items.size(), MAX_FAVORITES);
  for (size_t i = 0; i < count; ++i) {
    JsonDocument doc;
    doc["title"] = items[i].title;
    doc["link"] = items[i].link;
    doc["source"] = items[i].source;
    doc["published"] = items[i].published;
    doc["summary"] = items[i].summary;
    if (!writeJsonLine(file, doc)) {
      file.close();
      Storage.remove(temp.c_str());
      return false;
    }
  }

  file.flush();
  file.close();
  Storage.remove(FAVORITES_PATH);
  if (!Storage.rename(temp.c_str(), FAVORITES_PATH)) {
    Storage.remove(temp.c_str());
    return false;
  }
  return true;
}

bool isFavorite(const std::string& link) {
  std::vector<NewsArticle> items;
  if (!loadFavorites(items)) return false;
  return std::any_of(items.begin(), items.end(),
                     [&](const NewsArticle& item) { return item.link == link; });
}

bool toggleFavorite(const NewsArticle& article, bool* nowFavorite) {
  if (article.title.empty() || article.link.empty()) return false;

  std::vector<NewsArticle> items;
  loadFavorites(items);

  const auto it = std::find_if(
      items.begin(), items.end(),
      [&](const NewsArticle& item) { return item.link == article.link; });

  if (it != items.end()) {
    items.erase(it);
    if (!saveFavorites(items)) return false;
    if (nowFavorite) *nowFavorite = false;
    return true;
  }

  if (items.size() >= MAX_FAVORITES) return false;
  items.push_back(article);
  if (!saveFavorites(items)) return false;
  if (nowFavorite) *nowFavorite = true;
  return true;
}

bool writeArticleText(const NewsArticle& article) {
  if (!ensureRoot() || article.title.empty()) return false;

  std::string body;
  body.reserve(article.title.size() + article.summary.size() + article.link.size() + 256);
  body += article.title;
  body += "\n\n";
  if (!article.source.empty()) {
    body += "Zrodlo: ";
    body += article.source;
    body += "\n";
  }
  if (!article.published.empty()) {
    body += "Data: ";
    body += article.published;
    body += "\n";
  }
  if (!article.link.empty()) {
    body += "Link: ";
    body += article.link;
    body += "\n";
  }
  body += "\n----------------------------------------\n\n";
  body += article.summary.empty() ? "Brak tresci w kanale RSS/Atom." : article.summary;
  body += "\n";

  return Storage.writeFile(ARTICLE_PATH, String(body.c_str()));
}

}  // namespace NewsStore
