#include "PlannerStore.h"

#include <ArduinoJson.h>
#include <HalStorage.h>

#include <algorithm>
#include <cstdio>

namespace {
constexpr const char* MOD = "Planner";
constexpr size_t MAX_LINE_BYTES = 2048;

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

bool isLeap(const int y) {
  return (y % 4 == 0 && y % 100 != 0) || (y % 400 == 0);
}

int daysInMonth(const int y, const int m) {
  static const int DAYS[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
  if (m < 1 || m > 12) return 0;
  if (m == 2 && isLeap(y)) return 29;
  return DAYS[m - 1];
}

bool parseDate(const std::string& value, int& y, int& m, int& d) {
  if (value.size() != 10 || value[4] != '-' || value[7] != '-') return false;
  for (size_t i = 0; i < value.size(); ++i) {
    if (i == 4 || i == 7) continue;
    if (value[i] < '0' || value[i] > '9') return false;
  }
  y = std::stoi(value.substr(0, 4));
  m = std::stoi(value.substr(5, 2));
  d = std::stoi(value.substr(8, 2));
  if (y < 2000 || y > 2099) return false;
  if (m < 1 || m > 12) return false;
  return d >= 1 && d <= daysInMonth(y, m);
}

std::string formatDate(const int y, const int m, const int d) {
  char buf[16]{};
  std::snprintf(buf, sizeof(buf), "%04d-%02d-%02d", y, m, d);
  return std::string(buf);
}

bool ensureRoot() {
  if (!Storage.ensureDirectoryExists("/.x4-multihub")) return false;
  return Storage.ensureDirectoryExists(PlannerStore::ROOT);
}
}  // namespace

namespace PlannerStore {

bool validDate(const std::string& date) {
  int y = 0, m = 0, d = 0;
  return parseDate(date, y, m, d);
}

bool shiftDate(const std::string& input, const int deltaDays, std::string& output) {
  int y = 0, m = 0, d = 0;
  if (!parseDate(input, y, m, d)) return false;

  int remaining = deltaDays;
  while (remaining > 0) {
    ++d;
    if (d > daysInMonth(y, m)) {
      d = 1;
      ++m;
      if (m > 12) {
        m = 1;
        ++y;
      }
    }
    if (y > 2099) return false;
    --remaining;
  }
  while (remaining < 0) {
    --d;
    if (d < 1) {
      --m;
      if (m < 1) {
        m = 12;
        --y;
      }
      if (y < 2000) return false;
      d = daysInMonth(y, m);
    }
    ++remaining;
  }

  output = formatDate(y, m, d);
  return true;
}

bool load(std::vector<PlannerTask>& tasks) {
  tasks.clear();

  HalFile file;
  if (!Storage.openFileForRead(MOD, TASKS_PATH, file)) return false;

  std::string line;
  line.reserve(512);
  while (tasks.size() < MAX_TASKS && readLine(file, line)) {
    if (line.empty()) continue;

    JsonDocument doc;
    if (deserializeJson(doc, line)) continue;

    PlannerTask task;
    task.id = doc["id"] | 0U;
    task.date = doc["date"] | "";
    task.title = doc["title"] | "";
    task.note = doc["note"] | "";
    task.done = doc["done"] | false;
    task.priority = doc["priority"] | 2;

    if (task.id == 0 || !validDate(task.date) || task.title.empty()) continue;
    if (task.priority < 1 || task.priority > 3) task.priority = 2;
    tasks.push_back(std::move(task));
  }
  file.close();
  return true;
}

bool save(const std::vector<PlannerTask>& tasks) {
  if (!ensureRoot()) return false;

  const std::string temp = std::string(TASKS_PATH) + ".tmp";
  HalFile file;
  if (!Storage.openFileForWrite(MOD, temp, file)) return false;

  const size_t count = std::min(tasks.size(), MAX_TASKS);
  for (size_t i = 0; i < count; ++i) {
    const auto& task = tasks[i];
    JsonDocument doc;
    doc["id"] = task.id;
    doc["date"] = task.date;
    doc["title"] = task.title;
    doc["note"] = task.note;
    doc["done"] = task.done;
    doc["priority"] = task.priority;

    String out;
    serializeJson(doc, out);
    if (file.write(reinterpret_cast<const uint8_t*>(out.c_str()), out.length()) != out.length() ||
        file.write(static_cast<uint8_t>('\n')) != 1) {
      file.close();
      Storage.remove(temp.c_str());
      return false;
    }
  }

  file.flush();
  file.close();
  Storage.remove(TASKS_PATH);
  if (!Storage.rename(temp.c_str(), TASKS_PATH)) {
    Storage.remove(temp.c_str());
    return false;
  }
  return true;
}

bool loadActiveDate(std::string& date) {
  date.clear();
  HalFile file;
  if (!Storage.openFileForRead(MOD, ACTIVE_DATE_PATH, file)) return false;

  while (file.available() && date.size() < 16) {
    const int ch = file.read();
    if (ch < 0 || ch == '\n' || ch == '\r') break;
    date.push_back(static_cast<char>(ch));
  }
  file.close();

  if (!validDate(date)) {
    date.clear();
    return false;
  }
  return true;
}

bool saveActiveDate(const std::string& date) {
  if (!validDate(date) || !ensureRoot()) return false;
  return Storage.writeFile(ACTIVE_DATE_PATH, String(date.c_str()));
}

uint32_t nextId(const std::vector<PlannerTask>& tasks) {
  uint32_t maxId = 0;
  for (const auto& task : tasks) {
    if (task.id > maxId) maxId = task.id;
  }
  return maxId == UINT32_MAX ? 1 : maxId + 1;
}

}  // namespace PlannerStore
