/**
 * SPDX-FileComment: Semantic Versioning Utility
 * SPDX-FileType: SOURCE
 * SPDX-FileContributor: ZHENG Robert
 * SPDX-FileCopyrightText: 2026 ZHENG Robert
 * SPDX-License-Identifier: MIT
 *
 * @file semver.hpp
 * @brief Utilities for cleaning and parsing semantic version strings.
 * @version 1.0.0
 * @date 2026-02-18
 *
 * @author ZHENG Robert (robert@hase-zheng.net)
 * @copyright Copyright (c) 2026 ZHENG Robert
 *
 * @license MIT License
 */
#pragma once
#include <regex>
#include <string>

namespace depdiscover {

/**
 * @brief Cleans a version string to extract "Major.Minor.Patch".
 *
 * Removes prefixes like "v" and extracts the numeric version part.
 *
 * @param raw_version The raw version string.
 * @return std::string The cleaned version string, or original if no pattern matched.
 */
inline std::string clean_version(const std::string &raw_version) {
  if (raw_version.empty())
    return "";

  // Regex explanation:
  // (\\d+)       -> Major version (digit)
  // \\.          -> Dot
  // (\\d+)       -> Minor version (digit)
  // (?:\\.(\\d+))? -> Optional: Dot and Patch version
  //
  // std::regex_search finds the FIRST match in the string (ignores prefix/suffix)
  static const std::regex re(R"((\d+)\.(\d+)(?:\.(\d+))?)");
  std::smatch m;

  if (std::regex_search(raw_version, m, re)) {
    // Return complete match (e.g., "3.11.2" from "v3.11.2")
    return m[0].str();
  }

  // Fallback: if no version pattern is recognized (e.g., "latest", "system"),
  // return the original string.
  return raw_version;
}

/**
 * @brief Simple version comparison (not perfect semver, but enough for most cases).
 * 
 * @param v1 First version string.
 * @param v2 Second version string.
 * @return int 1 if v1 > v2, -1 if v1 < v2, 0 if equal.
 */
inline int compare_versions(const std::string &v1, const std::string &v2) {
  if (v1 == v2) return 0;
  if (v1.empty()) return -1;
  if (v2.empty()) return 1;

  std::string c1 = clean_version(v1);
  std::string c2 = clean_version(v2);

  std::stringstream ss1(c1), ss2(c2);
  std::string segment1, segment2;

  while (true) {
    bool has1 = (bool)std::getline(ss1, segment1, '.');
    bool has2 = (bool)std::getline(ss2, segment2, '.');

    if (!has1 && !has2) break;

    int n1 = 0, n2 = 0;
    try {
      if (has1) n1 = std::stoi(segment1);
      if (has2) n2 = std::stoi(segment2);
    } catch (...) {
      // If we can't parse as int, do string comparison for the segment
      if (segment1 > segment2) return 1;
      if (segment1 < segment2) return -1;
    }

    if (n1 > n2) return 1;
    if (n1 < n2) return -1;
  }

  return 0;
}

} // namespace depdiscover
