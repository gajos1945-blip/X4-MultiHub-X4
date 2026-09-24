#include "ReaderDashboardStore.h"

#include <CrossPointState.h>
#include <Epub.h>
#include <FsHelpers.h>
#include <HalStorage.h>
#include <RecentBooksStore.h>

#include <algorithm>
#include <cmath>
#include <functional>
#include <string>

namespace {
constexpr const char* MOD = "ReaderDash";

std::string filenameOf(const std::string& path) {
  const size_t slash = path.find_last_of('/');
  return slash == std::string::npos ? path : path.substr(slash + 1);
}

std::string epubCachePath(const std::string& path) {
  // Mirrors the cache-path formula used by pinned CrossPoint 1.6.0.
  return "/.crosspoint/epub_" +
         std::to_string(std::hash<std::string>{}(path));
}

int clampPercent(const int value) {
  return std::max(0, std::min(100, value));
}

void loadRecentMetadata(ReaderDashboardSnapshot& snapshot) {
  for (const auto& book : RECENT_BOOKS.getBooks()) {
    if (book.path != snapshot.path) continue;
    snapshot.title = book.title;
    snapshot.author = book.author;
    break;
  }

  if (snapshot.title.empty()) {
    snapshot.title = filenameOf(snapshot.path);
  }
}

void loadEpubProgress(ReaderDashboardSnapshot& snapshot) {
  const std::string fileName = filenameOf(snapshot.path);
  if (!FsHelpers::hasEpubExtension(fileName)) return;

  HalFile file;
  const std::string progressPath = epubCachePath(snapshot.path) + "/progress.bin";
  if (!Storage.openFileForRead(MOD, progressPath, file)) return;

  uint8_t data[10]{};
  const int size = file.read(data, sizeof(data));
  file.close();

  // CrossPoint 1.6.0 accepts the historical 4/6 byte formats plus the
  // current 10 byte format. We mirror that behavior and do not invent data
  // absent from an older cache.
  if (size != 4 && size != 6 && size != 10) return;

  snapshot.spineIndex =
      static_cast<uint16_t>(data[0] | (static_cast<uint16_t>(data[1]) << 8));
  snapshot.currentPage =
      static_cast<uint16_t>(data[2] | (static_cast<uint16_t>(data[3]) << 8));

  if (snapshot.currentPage == UINT16_MAX) return;
  snapshot.progressAvailable = true;

  if (size >= 6) {
    snapshot.chapterPageCount =
        static_cast<uint16_t>(data[4] | (static_cast<uint16_t>(data[5]) << 8));

    if (snapshot.chapterPageCount > 0) {
      const uint32_t pageNumber =
          std::min<uint32_t>(static_cast<uint32_t>(snapshot.currentPage) + 1U,
                             snapshot.chapterPageCount);
      snapshot.chapterProgressPercent = clampPercent(
          static_cast<int>((pageNumber * 100U +
                            snapshot.chapterPageCount / 2U) /
                           snapshot.chapterPageCount));
      snapshot.chapterProgressAvailable = true;
    }
  }

  // CrossPoint itself uses Epub(path, "/.crosspoint").load(false, true)
  // for lightweight cached metadata reads. buildIfMissing=false means this
  // dashboard does not trigger expensive indexing just to show progress.
  if (snapshot.chapterProgressAvailable) {
    Epub epub(snapshot.path, "/.crosspoint");
    if (epub.load(false, true) && epub.getBookSize() > 0) {
      const float chapter =
          static_cast<float>(snapshot.currentPage) /
          static_cast<float>(snapshot.chapterPageCount);
      const float book =
          epub.calculateProgress(snapshot.spineIndex, chapter) * 100.0f;

      snapshot.bookProgressPercent =
          clampPercent(static_cast<int>(std::lround(book)));
      snapshot.bookProgressAvailable = true;

      if (snapshot.title.empty()) snapshot.title = epub.getTitle();
      if (snapshot.author.empty()) snapshot.author = epub.getAuthor();
    }
  }
}
}  // namespace

namespace ReaderDashboardStore {

bool load(ReaderDashboardSnapshot& snapshot) {
  snapshot = {};

  snapshot.path = APP_STATE.openEpubPath;
  if (snapshot.path.empty()) return false;
  if (!Storage.exists(snapshot.path.c_str())) return false;

  snapshot.available = true;
  loadRecentMetadata(snapshot);
  loadEpubProgress(snapshot);
  return true;
}

std::string pageLabel(const ReaderDashboardSnapshot& snapshot) {
  if (!snapshot.progressAvailable) return "POSTEP UNKNOWN";

  if (snapshot.chapterProgressAvailable) {
    const uint32_t page =
        std::min<uint32_t>(static_cast<uint32_t>(snapshot.currentPage) + 1U,
                           snapshot.chapterPageCount);
    return "Rozdzial " + std::to_string(page) + "/" +
           std::to_string(snapshot.chapterPageCount) + " (" +
           std::to_string(snapshot.chapterProgressPercent) + "%)";
  }

  return "Strona " +
         std::to_string(static_cast<uint32_t>(snapshot.currentPage) + 1U);
}

std::string bookProgressLabel(const ReaderDashboardSnapshot& snapshot) {
  if (!snapshot.bookProgressAvailable) return "Ksiazka: UNKNOWN";
  return "Ksiazka: " + std::to_string(snapshot.bookProgressPercent) + "%";
}

}  // namespace ReaderDashboardStore
