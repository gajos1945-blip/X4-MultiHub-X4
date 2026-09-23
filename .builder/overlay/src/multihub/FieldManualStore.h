#pragma once

#include <cstddef>
#include <string>
#include <vector>

struct FieldManualEntry {
  std::string path;
  std::string title;
  std::string category;
  bool favorite = false;
  bool checklist = false;
};

namespace FieldManualStore {

constexpr size_t MAX_MANUALS = 300;
constexpr const char* MANUALS_ROOT = "/Manuals";
constexpr const char* STATE_ROOT = "/.x4-multihub/manuals";
constexpr const char* INDEX_PATH = "/.x4-multihub/manuals/index.jsonl";

bool load(std::vector<FieldManualEntry>& entries);
bool save(const std::vector<FieldManualEntry>& entries);
bool rebuild(std::vector<FieldManualEntry>& entries);
bool toggleFavorite(const std::string& path, bool* newValue = nullptr);

bool isChecklistPath(const std::string& path);

}  // namespace FieldManualStore
