/**
 * SPDX-FileComment: Conan Parser for Dependency Tracker
 * SPDX-FileType: SOURCE
 * SPDX-FileContributor: ZHENG Robert
 * SPDX-FileCopyrightText: 2026 ZHENG Robert
 * SPDX-License-Identifier: MIT
 *
 * @file conan_parser.hpp
 * @brief Parses conanfile.txt to identify dependencies.
 * @version 1.0.0
 * @date 2026-02-18
 *
 * @author ZHENG Robert (robert@hase-zheng.net)
 * @copyright Copyright (c) 2026 ZHENG Robert
 *
 * @license MIT License
 */
#pragma once
#include "semver.hpp" // cleanup versions
#include "types.hpp"  // Dependency struct
#include <fstream>
#include <regex>
#include <string>
#include <vector>

namespace depdiscover {

/**
 * @brief Parses a conanfile.txt to extract dependencies.
 *
 * @param path The path to the conanfile.txt.
 * @return std::vector<Dependency> A list of extracted dependencies.
 */
inline std::vector<Dependency>
parse_conan_dependencies(const std::string &path) {
  std::vector<Dependency> deps;
  std::ifstream f(path);
  if (!f)
    return deps;

  std::string line;
  bool in_requires_section = false;

  // Regex für: packetname/version (z.B. "fmt/8.1.1" oder
  // "zlib/1.2.11#revision") Group 1: Name, Group 2: Version
  std::regex re_dep(R"(^\s*([^/\s]+)/([^#\s\n\r]+))");

  // Regex für Sektions-Header [requires]
  std::regex re_section(R"(^\s*\[requires\]\s*)");

  while (std::getline(f, line)) {
    // 1. Prüfen, ob wir in der [requires] Sektion sind
    if (std::regex_match(line, re_section)) {
      in_requires_section = true;
      continue;
    }

    // Wenn wir eine neue Sektion erreichen (z.B. [generators]), hören wir auf
    if (line.find("[") == 0 && line.find("]") != std::string::npos &&
        line.find("[requires]") == std::string::npos) {
      in_requires_section = false;
    }

    if (in_requires_section) {
      std::smatch m;
      if (std::regex_search(line, m, re_dep)) {
        Dependency d;
        d.name = m[1];
        d.version = clean_version(m[2]);
        d.type = "conan";
        deps.push_back(d);
      }
    }
  }

  return deps;
}

} // namespace depdiscover