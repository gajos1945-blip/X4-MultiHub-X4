#pragma once

#include <cstdint>
#include <string>
#include <vector>

struct PlannerTask {
  uint32_t id = 0;
  std::string date;
  std::string title;
  std::string note;
  bool done = false;
  uint8_t priority = 2;  // 1=high, 2=normal, 3=low
};

namespace PlannerStore {

constexpr size_t MAX_TASKS = 400;
constexpr const char* ROOT = "/.x4-multihub/planner";
constexpr const char* TASKS_PATH = "/.x4-multihub/planner/tasks.jsonl";
constexpr const char* ACTIVE_DATE_PATH = "/.x4-multihub/planner/active_date.txt";

bool load(std::vector<PlannerTask>& tasks);
bool save(const std::vector<PlannerTask>& tasks);

bool loadActiveDate(std::string& date);
bool saveActiveDate(const std::string& date);

bool validDate(const std::string& date);
bool shiftDate(const std::string& input, int deltaDays, std::string& output);

uint32_t nextId(const std::vector<PlannerTask>& tasks);

}  // namespace PlannerStore
