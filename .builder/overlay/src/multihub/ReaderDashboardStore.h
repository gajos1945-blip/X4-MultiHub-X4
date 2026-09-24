#pragma once

#include <cstdint>
#include <string>

struct ReaderDashboardSnapshot {
  bool available = false;
  bool progressAvailable = false;
  bool chapterProgressAvailable = false;
  bool bookProgressAvailable = false;

  std::string path;
  std::string title;
  std::string author;

  uint16_t spineIndex = 0;
  uint16_t currentPage = 0;       // zero based, mirrors CrossPoint progress.bin
  uint16_t chapterPageCount = 0;

  int chapterProgressPercent = 0;
  int bookProgressPercent = 0;
};

namespace ReaderDashboardStore {

// Reads CrossPoint's own state/recent/progress cache.
// It never modifies CrossPoint progress files.
bool load(ReaderDashboardSnapshot& snapshot);

std::string pageLabel(const ReaderDashboardSnapshot& snapshot);
std::string bookProgressLabel(const ReaderDashboardSnapshot& snapshot);

}  // namespace ReaderDashboardStore
