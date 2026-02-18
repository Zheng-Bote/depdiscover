/**
 * SPDX-FileComment: Header File Resolver
 * SPDX-FileType: SOURCE
 * SPDX-FileContributor: ZHENG Robert
 * SPDX-FileCopyrightText: 2026 ZHENG Robert
 * SPDX-License-Identifier: MIT
 *
 * @file header_resolver.hpp
 * @brief Scans include directives and resolves them to absolute paths.
 * @version 1.0.0
 * @date 2026-02-18
 *
 * @author ZHENG Robert (robert@hase-zheng.net)
 * @copyright Copyright (c) 2026 ZHENG Robert
 *
 * @license MIT License
 */
#pragma once
#include <filesystem>
#include <fstream>
#include <iostream>
#include <regex>
#include <string>
#include <vector>

namespace depdiscover {

namespace fs = std::filesystem;

/**
 * @brief Scans a source file for `#include` directives.
 *
 * @param source_file The path to the source file.
 * @return std::vector<std::string> A list of included header names.
 */
inline std::vector<std::string> scan_includes(const std::string &source_file) {
  std::vector<std::string> included_headers;
  if (!fs::exists(source_file))
    return {};

  std::ifstream f(source_file);
  if (!f)
    return {};

  static const std::regex re(R"(\s*#\s*include\s*[<"]([^>"]+)[>"])");
  std::string line;
  while (std::getline(f, line)) {
    std::smatch m;
    if (std::regex_search(line, m, re)) {
      included_headers.push_back(m[1].str());
    }
  }
  return included_headers;
}

// work_dir: Das Verzeichnis, in dem der Compiler ausgeführt wurde
/**
 * @brief Resolves a header filename to an absolute path.
 *
 * Checks absolute paths, explicit include paths, and system paths.
 *
 * @param header_name The name of the header file.
 * @param include_paths A list of include directories.
 * @param work_dir The compiler's working directory (for relative include
 * paths).
 * @return std::string The absolute path to the header, or empty if not found.
 */
inline std::string resolve_header(const std::string &header_name,
                                  const std::vector<std::string> &include_paths,
                                  const std::string &work_dir = "") {

  fs::path p_header(header_name);

  // 1. Ist der Header-Name selbst schon absolut?
  if (p_header.is_absolute()) {
    std::error_code ec;
    if (fs::exists(p_header, ec))
      return fs::canonical(p_header, ec).string();
    return "";
  }

  // Helper: Pfad prüfen und kanonisieren
  auto check_path = [&](const fs::path &full_p) -> std::string {
    std::error_code ec;
    if (fs::exists(full_p, ec))
      return fs::canonical(full_p, ec).string();
    return "";
  };

  // 2. Explizite Include-Pfade (-I) durchsuchen
  for (const auto &inc_str : include_paths) {
    fs::path inc_path(inc_str);

    // Wenn der -I Pfad relativ ist und wir ein work_dir haben, verknüpfen!
    if (inc_path.is_relative() && !work_dir.empty()) {
      inc_path = fs::path(work_dir) / inc_path;
    }

    std::string res = check_path(inc_path / p_header);
    if (!res.empty())
      return res;
  }

  // 3. Systempfade (Fallback) - Wichtig für exiv2 in /usr/local/include
  static const std::vector<std::string> system_paths = {
      "/usr/include", "/usr/local/include", "/usr/include/x86_64-linux-gnu",
      "/opt/local/include"};

  for (const auto &sys_path : system_paths) {
    std::string res = check_path(fs::path(sys_path) / p_header);
    if (!res.empty())
      return res;
  }

  return "";
}

} // namespace depdiscover