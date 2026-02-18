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
 * @return std::string The cleaned version string, or original if no pattern
 * matched.
 */
inline std::string clean_version(const std::string &raw_version) {
  if (raw_version.empty())
    return "";

  // Regex Erklärung:
  // (\d+)       -> Hauptversion (Zahl)
  // \.          -> Punkt
  // (\d+)       -> Nebenversion (Zahl)
  // (?:\.(\d+))? -> Optional: Punkt und Patch-Version
  //
  // std::regex_search sucht den ERSTEN Treffer im String (ignoriert
  // Prefix/Suffix)
  static const std::regex re(R"((\d+)\.(\d+)(?:\.(\d+))?)");
  std::smatch m;

  if (std::regex_search(raw_version, m, re)) {
    // Gibt den kompletten Match zurück (z.B. "3.11.2" aus "v3.11.2")
    return m[0].str();
  }

  // Fallback: Wenn kein Versions-Muster erkannt wird (z.B. "latest", "system"),
  // geben wir den Original-String zurück.
  return raw_version;
}

} // namespace depdiscover