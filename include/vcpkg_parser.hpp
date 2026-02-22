/**
 * SPDX-FileComment: Vcpkg Manifest Parser
 * SPDX-FileType: SOURCE
 * SPDX-FileContributor: ZHENG Robert
 * SPDX-FileCopyrightText: 2026 ZHENG Robert
 * SPDX-License-Identifier: MIT
 *
 * @file vcpkg_parser.hpp
 * @brief Parses vcpkg.json manifests to extract dependencies.
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
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

namespace depdiscover {

/**
 * @brief Parses a vcpkg.json manifest file.
 *
 * @param manifest_path Path to the vcpkg.json file.
 * @return std::vector<Dependency> List of extracted dependencies.
 */
inline std::vector<Dependency>
parse_vcpkg_manifest(const std::string &manifest_path) {
  std::vector<Dependency> deps;
  try {
    std::ifstream f(manifest_path);
    if (!f.is_open())
      return deps;

    nlohmann::json j;
    f >> j;

    if (j.contains("dependencies") && j["dependencies"].is_array()) {
      for (const auto &item : j["dependencies"]) {
        Dependency dep;
        dep.type = "vcpkg";
        dep.source = "manifest";

        // vcpkg dependencies can be simple strings or complex objects
        if (item.is_string()) {
          dep.name = item.get<std::string>();
          dep.version = "unknown";
        } else if (item.is_object()) {
          dep.name = item.value("name", "unknown");

          // Try to extract version constraints
          if (item.contains("version>=")) {
            dep.version = item["version>="];
          } else if (item.contains("version")) {
            dep.version = item["version"];
          } else {
            dep.version = "unknown";
          }
        }

        if (dep.name != "unknown") {
          deps.push_back(dep);
        }
      }
    }
  } catch (const std::exception &e) {
    std::cerr << "[Warning] Could not parse vcpkg manifest: " << e.what()
              << "\n";
  }
  return deps;
}

} // namespace depdiscover
