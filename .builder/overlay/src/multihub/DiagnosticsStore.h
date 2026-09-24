#pragma once

#include <string>

namespace DiagnosticsStore {

constexpr const char* ROOT = "/.x4-multihub/diagnostics";
constexpr const char* REPORT_PATH =
    "/.x4-multihub/diagnostics/last_report.txt";

bool storageWriteTest();
bool writeReport(const std::string& report);

}  // namespace DiagnosticsStore
