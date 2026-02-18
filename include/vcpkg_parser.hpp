/**
 * SPDX-FileComment: Vcpkg Manifest Parser
 * SPDX-FileType: SOURCE
 * SPDX-FileContributor: ZHENG Robert
 * SPDX-FileCopyrightText: 2026 ZHENG Robert
 * SPDX-License-Identifier: MIT
 *
 * @file vcpkg_parser.hpp
 * @brief Parses vcpkg.json manifests to identify dependencies.
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
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

namespace depdiscover {

// ENTFERNT: struct Dependency { ... }
// ENTFERNT: void to_json(...) { ... }
// Beide sind jetzt in types.hpp definiert.

/**
 * @brief Parses a vcpkg.json manifest file.
 *
 * @param path The path to the vcpkg.json file.
 * @return std::vector<Dependency> A list of dependencies from the manifest.
 */
inline std::vector<Dependency> parse_vcpkg_manifest(const std::string &path) {
  std::vector<Dependency> deps;
  std::ifstream f(path);
  if (!f)
    return deps;

  nlohmann::json doc;
  try {
    f >> doc;
  } catch (...) {
    return deps;
  }

  // Vcpkg Manifest hat "dependencies": [ ... ]
  if (doc.contains("dependencies") && doc["dependencies"].is_array()) {
    for (const auto &entry : doc["dependencies"]) {
      Dependency dep;
      dep.type = "vcpkg";

      if (entry.is_string()) {
        // Fall 1: "dependencies": ["fmt", "zlib"]
        dep.name = entry.get<std::string>();
        dep.version = "latest";
      } else if (entry.is_object()) {
        // Fall 2: "dependencies": [ { "name": "fmt", "version>=": "10.0" } ]
        dep.name = entry.value("name", "unknown");

        // Versuche verschiedene Versions-Felder
        if (entry.contains("version-string"))
          dep.version = entry["version-string"];
        else if (entry.contains("version>="))
          dep.version = entry["version>="];
        else if (entry.contains("version"))
          dep.version = entry["version"];
        else
          dep.version = "latest";
      }
      dep.version = clean_version(dep.version);

      if (!dep.name.empty()) {
        deps.push_back(dep);
      }
    }
  }
  return deps;
}

} // namespace depdiscover