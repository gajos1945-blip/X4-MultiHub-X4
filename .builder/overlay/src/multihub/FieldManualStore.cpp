#include "FieldManualStore.h"

#include <ArduinoJson.h>
#include <FsHelpers.h>
#include <HalStorage.h>

#include <algorithm>
#include <cctype>
#include <cstring>
#include <utility>

namespace {
constexpr const char* MOD = "FieldManual";
constexpr size_t NAME_BUFFER_SIZE = 384;
constexpr size_t MAX_LINE_BYTES = 1536;

std::string lowerAscii(std::string value) {
  for (char& c : value) {
    const unsigned char u = static_cast<unsigned char>(c);
    if (u < 128) c = static_cast<char>(std::tolower(u));
  }
  return value;
}

std::string filenameOf(const std::string& path) {
  const size_t slash = path.find_last_of('/');
  return slash == std::string::npos ? path : path.substr(slash + 1);
}

std::string stemOf(const std::string& path) {
  std::string name = filenameOf(path);
  const size_t dot = name.find_last_of('.');
  if (dot != std::string::npos) name.resize(dot);
  return name;
}

std::string categoryOf(const std::string& path) {
  constexpr const char* prefix = "/Manuals/";
  if (path.rfind(prefix, 0) != 0) return "Ogolne";

  const size_t start = std::strlen(prefix);
  const size_t slash = path.find('/', start);
  if (slash == std::string::npos || slash == start) return "Ogolne";
  return path.substr(start, slash - start);
}

bool supportedManual(const std::string& path) {
  return FsHelpers::hasTxtExtension(path) ||
         FsHelpers::hasMarkdownExtension(path) ||
         FsHelpers::hasEpubExtension(path) ||
         FsHelpers::hasXtcExtension(path) ||
         FieldManualStore::isChecklistPath(path);
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

bool parseEntryLine(const std::string& line, FieldManualEntry& entry) {
  if (line.empty()) return false;
  JsonDocument doc;
  if (deserializeJson(doc, line)) return false;

  const char* path = doc["path"] | nullptr;
  if (!path || !*path) return false;

  entry.path = path;
  entry.title = doc["title"] | "";
  entry.category = doc["category"] | "Ogolne";
  entry.favorite = doc["favorite"] | false;
  entry.checklist = doc["checklist"] | false;
  return true;
}

bool writeEntryLine(HalFile& file, const FieldManualEntry& entry) {
  JsonDocument doc;
  doc["path"] = entry.path;
  doc["title"] = entry.title;
  doc["category"] = entry.category;
  doc["favorite"] = entry.favorite;
  doc["checklist"] = entry.checklist;

  String out;
  serializeJson(doc, out);
  if (file.write(reinterpret_cast<const uint8_t*>(out.c_str()), out.length()) != out.length()) {
    return false;
  }
  return file.write(static_cast<uint8_t>('\n')) == 1;
}

bool oldFavorite(const std::vector<FieldManualEntry>& oldEntries,
                 const std::string& path) {
  const auto it = std::find_if(
      oldEntries.begin(), oldEntries.end(),
      [&](const FieldManualEntry& item) { return item.path == path; });
  return it != oldEntries.end() && it->favorite;
}

}  // namespace

namespace FieldManualStore {

bool isChecklistPath(const std::string& path) {
  const std::string lower = lowerAscii(path);
  constexpr const char* suffix = ".checklist";
  return lower.size() >= std::strlen(suffix) &&
         lower.compare(lower.size() - std::strlen(suffix), std::strlen(suffix), suffix) == 0;
}

bool load(std::vector<FieldManualEntry>& entries) {
  entries.clear();

  HalFile file;
  if (!Storage.openFileForRead(MOD, INDEX_PATH, file)) return false;

  std::string line;
  line.reserve(384);
  while (entries.size() < MAX_MANUALS && readLine(file, line)) {
    FieldManualEntry entry;
    if (!parseEntryLine(line, entry)) continue;
    if (!Storage.exists(entry.path.c_str())) continue;
    entries.push_back(std::move(entry));
  }
  file.close();
  return true;
}

bool save(const std::vector<FieldManualEntry>& entries) {
  if (!Storage.ensureDirectoryExists("/.x4-multihub")) return false;
  if (!Storage.ensureDirectoryExists(STATE_ROOT)) return false;

  const std::string temp = std::string(INDEX_PATH) + ".tmp";
  HalFile file;
  if (!Storage.openFileForWrite(MOD, temp, file)) return false;

  const size_t count = std::min(entries.size(), MAX_MANUALS);
  for (size_t i = 0; i < count; ++i) {
    if (!writeEntryLine(file, entries[i])) {
      file.close();
      Storage.remove(temp.c_str());
      return false;
    }
  }

  file.flush();
  file.close();

  Storage.remove(INDEX_PATH);
  if (!Storage.rename(temp.c_str(), INDEX_PATH)) {
    Storage.remove(temp.c_str());
    return false;
  }
  return true;
}

bool rebuild(std::vector<FieldManualEntry>& entries) {
  std::vector<FieldManualEntry> previous;
  load(previous);

  if (!Storage.exists(MANUALS_ROOT)) {
    if (!Storage.ensureDirectoryExists(MANUALS_ROOT)) return false;
  }

  entries.clear();
  entries.reserve(64);

  std::vector<std::string> pendingDirs;
  pendingDirs.reserve(16);
  pendingDirs.push_back(MANUALS_ROOT);

  char nameBuffer[NAME_BUFFER_SIZE]{};

  while (!pendingDirs.empty() && entries.size() < MAX_MANUALS) {
    std::string dirPath = std::move(pendingDirs.back());
    pendingDirs.pop_back();

    auto dir = Storage.open(dirPath.c_str());
    if (!dir || !dir.isDirectory()) continue;
    dir.rewindDirectory();

    for (auto entry = dir.openNextFile();
         entry && entries.size() < MAX_MANUALS;
         entry = dir.openNextFile()) {
      entry.getName(nameBuffer, sizeof(nameBuffer));
      if (nameBuffer[0] == '\0' ||
          std::strcmp(nameBuffer, ".") == 0 ||
          std::strcmp(nameBuffer, "..") == 0 ||
          std::strcmp(nameBuffer, "System Volume Information") == 0) {
        continue;
      }

      const bool isDir = entry.isDirectory();
      entry.close();

      std::string full = dirPath;
      if (full.empty() || full.back() != '/') full += '/';
      full += nameBuffer;

      if (isDir) {
        pendingDirs.push_back(std::move(full));
        continue;
      }

      if (!supportedManual(full)) continue;

      FieldManualEntry item;
      item.path = std::move(full);
      item.title = stemOf(item.path);
      item.category = categoryOf(item.path);
      item.favorite = oldFavorite(previous, item.path);
      item.checklist = isChecklistPath(item.path);
      entries.push_back(std::move(item));
    }
    dir.close();
  }

  std::sort(entries.begin(), entries.end(),
            [](const FieldManualEntry& a, const FieldManualEntry& b) {
              if (a.category != b.category) {
                return FsHelpers::naturalLess(a.category, b.category);
              }
              return FsHelpers::naturalLess(a.title, b.title);
            });

  return save(entries);
}

bool toggleFavorite(const std::string& path, bool* newValue) {
  std::vector<FieldManualEntry> entries;
  if (!load(entries)) return false;

  for (auto& entry : entries) {
    if (entry.path != path) continue;
    entry.favorite = !entry.favorite;
    const bool value = entry.favorite;
    if (!save(entries)) return false;
    if (newValue) *newValue = value;
    return true;
  }
  return false;
}

}  // namespace FieldManualStore
