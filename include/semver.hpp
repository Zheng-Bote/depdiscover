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

} // namespace depdiscover
