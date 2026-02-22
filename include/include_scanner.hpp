/**
 * SPDX-FileComment: Include Path and Library Scanner
 * SPDX-FileType: SOURCE
 * SPDX-FileContributor: ZHENG Robert
 * SPDX-FileCopyrightText: 2026 ZHENG Robert
 * SPDX-License-Identifier: MIT
 *
 * @file include_scanner.hpp
 * @brief Scans compiler flags for include paths and library names.
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
#include <vector>

namespace depdiscover {

/**
 * @brief Extracts include paths (-I flag) from a compiler command string.
 *
 * @param cmd The compiler command string.
 * @return std::vector<std::string> A list of extracted include paths.
 */
inline std::vector<std::string> extract_include_paths(const std::string &cmd) {
  std::vector<std::string> out;
  // Fix: \\s* allows spaces (e.g., "-I /usr/include")
  // ([^\\s]+) captures everything until the next whitespace
  std::regex re("-I\\s*([^\\s]+)");
  std::smatch m;

  auto begin = cmd.cbegin();
  auto end = cmd.cend();

  while (std::regex_search(begin, end, m, re)) {
    out.push_back(m[1]);
    begin = m.suffix().first;
  }

  return out;
}

/**
 * @brief Extracts library names (-l flag) from a compiler command string.
 *
 * @param cmd The compiler command string.
 * @return std::vector<std::string> A list of extracted library names.
 */
inline std::vector<std::string> extract_libraries(const std::string &cmd) {
  std::vector<std::string> out;
  // Same fix for libraries (-l lib vs -llib)
  std::regex re("-l\\s*([^\\s]+)");
  std::smatch m;

  auto begin = cmd.cbegin();
  auto end = cmd.cend();

  while (std::regex_search(begin, end, m, re)) {
    out.push_back(m[1]);
    begin = m.suffix().first;
  }

  return out;
}

} // namespace depdiscover
