#include "DiagnosticsStore.h"

#include <HalStorage.h>

#include <string>

namespace {
constexpr const char* MOD = "Diagnostics";
constexpr const char* TEST_PATH =
    "/.x4-multihub/diagnostics/.write_test.tmp";

bool ensureRoot() {
  if (!Storage.ensureDirectoryExists("/.x4-multihub")) return false;
  return Storage.ensureDirectoryExists(DiagnosticsStore::ROOT);
}

bool readWhole(const char* path, std::string& raw) {
  raw.clear();

  HalFile file;
  if (!Storage.openFileForRead(MOD, path, file)) return false;

  while (file.available() && raw.size() < 256) {
    const int ch = file.read();
    if (ch < 0) break;
    raw.push_back(static_cast<char>(ch));
  }

  const bool tooLarge = file.available();
  file.close();
  return !tooLarge;
}
}  // namespace

namespace DiagnosticsStore {

bool storageWriteTest() {
  if (!ensureRoot()) return false;

  const String expected = "x4-multihub-diagnostics\n";
  Storage.remove(TEST_PATH);

  if (!Storage.writeFile(TEST_PATH, expected)) return false;

  std::string actual;
  const bool readOk = readWhole(TEST_PATH, actual);
  const bool removeOk = Storage.remove(TEST_PATH);

  return readOk && removeOk && actual == expected.c_str();
}

bool writeReport(const std::string& report) {
  if (!ensureRoot() || report.empty() || report.size() > 8192) return false;

  const std::string temp = std::string(REPORT_PATH) + ".tmp";
  if (!Storage.writeFile(temp.c_str(), report.c_str())) return false;

  Storage.remove(REPORT_PATH);
  if (!Storage.rename(temp.c_str(), REPORT_PATH)) {
    Storage.remove(temp.c_str());
    return false;
  }

  return true;
}

}  // namespace DiagnosticsStore
