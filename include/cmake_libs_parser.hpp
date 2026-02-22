/**
 * SPDX-FileComment: CMake Libraries Parser for Dependency Tracker
 * SPDX-FileType: SOURCE
 * SPDX-FileContributor: ZHENG Robert
 * SPDX-FileCopyrightText: 2026 ZHENG Robert
 * SPDX-License-Identifier: MIT
 *
 * @file cmake_libs_parser.hpp
 * @brief Parses CMake libs.txt and fetches metadata from build directories.
 * @version 1.0.0
 * @date 2026-02-18
 *
 * @author ZHENG Robert (robert@hase-zheng.net)
 * @copyright Copyright (c) 2026 ZHENG Robert
 *
 * @license MIT License
 */
#pragma once
#include "types.hpp"
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <regex>
#include <string>
#include <vector>

namespace depdiscover {

namespace fs = std::filesystem;

/**
 * @brief Reads a version string from a header file using a regex pattern.
 *
 * @param path The path to the header file.
 * @param regex_pattern The regex pattern to extract version information.
 * @return std::string The extracted version string, or empty if not found.
 */
inline std::string read_header_version(const fs::path &path,
                                       const std::string &regex_pattern) {
  std::ifstream f(path);
  if (!f)
    return "";

  std::string content((std::istreambuf_iterator<char>(f)),
                      std::istreambuf_iterator<char>());
  std::regex re(regex_pattern);
  std::smatch m;

  if (std::regex_search(content, m, re)) {
    // Case 1: Major.Minor.Patch (3 groups)
    if (m.size() >= 4) {
      return m[1].str() + "." + m[2].str() + "." + m[3].str();
    }
    // Case 2: Single integer (e.g., FMT_VERSION 100100)
    if (m.size() == 2) {
      return m[1].str(); // Return raw, caller handles formatting
    }
  }
  return "";
}

/**
 * @brief Fetches metadata (version, license) for a CMake target from build
 * directories.
 *
 * Searches in `_deps` (FetchContent) and `vcpkg_installed` directories.
 *
 * @param target_name The name of the CMake target.
 * @param build_dir The CMake build directory.
 * @return std::pair<std::string, std::string> A pair containing {version,
 * license}.
 */
inline std::pair<std::string, std::string>
fetch_cmake_metadata(const std::string &target_name,
                     const fs::path &build_dir) {
  std::string clean = target_name;
  // Remove namespace (Qt6::Core -> Core, fmt::fmt -> fmt)
  if (clean.find("::") != std::string::npos) {
    // Take the part BEFORE :: for libraries like nlohmann_json::nlohmann_json -> nlohmann_json
    // or the part AFTER for Qt6::Core -> Core? Usually the package name before :: is more
    // relevant for the directory structure (nlohmann_json). We try both implicitly.
    clean = clean.substr(0, clean.find("::"));
  }
  std::transform(clean.begin(), clean.end(), clean.begin(), ::tolower);

  // Locations where CMake stores dependencies
  std::vector<fs::path> search_dirs = {
      build_dir / "_deps",
      build_dir / "vcpkg_installed" / "x64-linux" / "share" // Vcpkg Linux Standard
  };

  // 1. Check FetchContent (_deps)
  fs::path deps_dir = build_dir / "_deps";
  if (fs::exists(deps_dir)) {
    // nlohmann_json special case
    if (clean.find("nlohmann") != std::string::npos ||
        clean.find("json") != std::string::npos) {
      // _deps/json-src/include/nlohmann/json.hpp
      fs::path header =
          deps_dir / "json-src" / "include" / "nlohmann" / "json.hpp";
      if (fs::exists(header)) {
        std::string v = read_header_version(
            header,
            R"(#define\s+NLOHMANN_JSON_VERSION_MAJOR\s+(\d+).*?#define\s+NLOHMANN_JSON_VERSION_MINOR\s+(\d+).*?#define\s+NLOHMANN_JSON_VERSION_PATCH\s+(\d+))");
        if (!v.empty())
          return {v, "MIT"};
      }
    }
    // fmt special case
    if (clean == "fmt") {
      // _deps/fmt-src/include/fmt/core.h
      fs::path header = deps_dir / "fmt-src" / "include" / "fmt" / "core.h";
      if (fs::exists(header)) {
        std::string v_raw =
            read_header_version(header, R"(#define\s+FMT_VERSION\s+(\d+))");
        if (!v_raw.empty()) {
          try {
            int v_int = std::stoi(v_raw);
            std::string v_fmt = std::to_string(v_int / 10000) + "." +
                                std::to_string((v_int % 10000) / 100) + "." +
                                std::to_string(v_int % 100);
            return {v_fmt, "MIT"};
          } catch (...) {
          }
        }
      }
    }
  }

  // 2. Check Vcpkg Manifests (share/package/vcpkg.json)
  // Search recursively or directly
  fs::path vcpkg_share = build_dir / "vcpkg_installed" / "x64-linux" / "share";
  if (fs::exists(vcpkg_share / clean / "vcpkg.json")) {
    try {
      std::ifstream f(vcpkg_share / clean / "vcpkg.json");
      nlohmann::json j;
      f >> j;
      std::string ver =
          j.value("version-string", j.value("version", "unknown"));
      std::string lic = j.value("license", "unknown");
      return {ver, lic};
    } catch (...) {
    }
  }

  return {"unknown", "unknown"};
}

/**
 * @brief Parses the libs.txt file generated by CMake to identify dependencies.
 *
 * @param libs_txt_path The path to the libs.txt file.
 * @return std::vector<Dependency> A list of identified dependencies.
 */
inline std::vector<Dependency>
parse_cmake_libs(const std::string &libs_txt_path) {
  std::vector<Dependency> deps;
  std::ifstream f(libs_txt_path);
  if (!f)
    return deps;

  std::string content;
  std::getline(f, content); // libs.txt is usually a single line with ';' separator

  if (content.empty())
    return deps;

  std::stringstream ss(content);
  std::string segment;

  // Guess build directory (parent directory of libs.txt)
  fs::path build_dir = fs::path(libs_txt_path).parent_path();

  while (std::getline(ss, segment, ';')) {
    if (segment.empty())
      continue;

    // Ignore absolute paths (e.g., /usr/lib/libfoo.so) - those are handled via ldd/elf_scanner
    if (segment.find("/") != std::string::npos)
      continue;

    // Ignore flags (-lfoo) - those are handled via include_scanner
    if (segment[0] == '-')
      continue;

    // We are interested in CMake targets: "Qt6::Core", "fmt::fmt", "nlohmann_json"
    std::string name = segment;
    std::string version = "unknown";
    std::string source = "cmake_target";
    std::vector<std::string> licenses;

    // Fetch metadata
    auto [meta_ver, meta_lic] = fetch_cmake_metadata(name, build_dir);

    if (meta_ver != "unknown") {
      version = meta_ver;
      source = "cmake_fetchcontent/vcpkg"; // Upgrade source info
    }

    if (meta_lic != "unknown") {
      licenses.push_back(meta_lic);
    }

    // Special treatment for Qt (if qmake/qt is not available, we guess or leave as unknown)
    if (name.rfind("Qt", 0) == 0 || name.rfind("Qt6::", 0) == 0) {
      if (version == "unknown")
        version = "System/Qt"; // Placeholder
      if (licenses.empty())
        licenses = {"LGPL-3.0"};
    }

    Dependency d;
    d.name = name;
    d.version = version;
    d.type = "cmake_target";
    d.source = source;
    d.licenses = licenses;

    deps.push_back(d);
  }

  return deps;
}

} // namespace depdiscover
