/**
 * SPDX-FileComment: Pkg-Config Wrapper
 * SPDX-FileType: SOURCE
 * SPDX-FileContributor: ZHENG Robert
 * SPDX-FileCopyrightText: 2026 ZHENG Robert
 * SPDX-License-Identifier: MIT
 *
 * @file pkg_config.hpp
 * @brief Wrapper class for querying pkg-config.
 * @version 1.0.0
 * @date 2026-02-18
 *
 * @author ZHENG Robert (robert@hase-zheng.net)
 * @copyright Copyright (c) 2026 ZHENG Robert
 *
 * @license MIT License
 */
#pragma once
#include <array>
#include <cstdio>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

namespace depdiscover {

/**
 * @brief Holds information returned by pkg-config.
 */
struct PkgInfo {
  bool found = false;                       ///< True if the package was found.
  std::string version;                      ///< The version of the package.
  std::vector<std::string> include_paths;    ///< List of include paths (-I).
  std::vector<std::string> lib_names;       ///< List of library names (-l).
};

/**
 * @brief Static class for querying pkg-config.
 */
class PkgConfig {
public:
  /**
   * @brief Queries pkg-config for a specific package.
   *
   * @param package_name The name of the package.
   * @return PkgInfo The result of the query.
   */
  static PkgInfo query(const std::string &package_name) {
    PkgInfo info;

    // Retrieve version
    std::string ver = exec("pkg-config --modversion " + package_name);
    if (!ver.empty()) {
      // Remove trailing newline
      while (!ver.empty() && (ver.back() == '\n' || ver.back() == '\r'))
        ver.pop_back();
      info.version = ver;
      info.found = true;
    } else {
      return info; // Package not found
    }

    // Include paths (-I)
    std::string cflags = exec("pkg-config --cflags-only-I " + package_name);
    info.include_paths = parse_flags(cflags, "-I");

    // Lib names (-l)
    std::string libs = exec("pkg-config --libs-only-l " + package_name);
    info.lib_names = parse_flags(libs, "-l");

    return info;
  }

private:
  /**
   * @brief Executes a shell command and returns the output.
   *
   * @param cmd The command to execute.
   * @return std::string The command's stdout.
   */
  static std::string exec(const std::string &cmd) {
    std::array<char, 256> buffer;
    std::string result;
    // Discard stderr
    std::unique_ptr<FILE, decltype(&pclose)> pipe(
        popen((cmd + " 2>/dev/null").c_str(), "r"), pclose);
    if (!pipe)
      return "";
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
      result += buffer.data();
    }
    return result;
  }

  /**
   * @brief Parses a string of compiler flags.
   *
   * @param input The flags string.
   * @param prefix The prefix to look for (e.g., "-I").
   * @return std::vector<std::string> The extracted values.
   */
  static std::vector<std::string> parse_flags(const std::string &input,
                                              const std::string &prefix) {
    std::vector<std::string> out;
    std::stringstream ss(input);
    std::string segment;

    // Split at whitespace
    while (ss >> segment) {
      if (segment.rfind(prefix, 0) == 0) { // Starts with prefix (e.g., -I)
        std::string val = segment.substr(prefix.length());
        if (!val.empty())
          out.push_back(val);
      }
    }
    return out;
  }
};

} // namespace depdiscover
