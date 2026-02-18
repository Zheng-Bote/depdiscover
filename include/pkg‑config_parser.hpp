/**
 * SPDX-FileComment: Vcpkg Manifest Parser (Misnamed?)
 * SPDX-FileType: SOURCE
 * SPDX-FileContributor: ZHENG Robert
 * SPDX-FileCopyrightText: 2026 ZHENG Robert
 * SPDX-License-Identifier: MIT
 *
 * @file pkg‑config_parser.hpp
 * @brief Parses vcpkg.json manifests (appears to be a duplicate/misnamed file).
 * @version 1.0.0
 * @date 2026-02-18
 *
 * @author ZHENG Robert (robert@hase-zheng.net)
 * @copyright Copyright (c) 2026 ZHENG Robert
 *
 * @license MIT License
 */
#pragma once
#include "types.hpp" // WICHTIG: Include statt struct Definition
#include <fstream>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

namespace depdiscover {

// struct Dependency wurde nach types.hpp verschoben!

/**
 * @brief Parses a vcpkg.json manifest file.
 *
 * @param path The path to the vcpkg.json file.
 * @return std::vector<Dependency> A list of dependencies suitable for
 * pkg-config (?).
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

  if (doc.contains("dependencies") && doc["dependencies"].is_array()) {
    for (const auto &entry : doc["dependencies"]) {
      Dependency dep;
      dep.type = "vcpkg";

      if (entry.is_string()) {
        dep.name = entry.get<std::string>();
        dep.version = "latest";
      } else if (entry.is_object()) {
        dep.name = entry.value("name", "unknown");

        if (entry.contains("version-string"))
          dep.version = entry["version-string"];
        else if (entry.contains("version>="))
          dep.version = entry["version>="];
        else if (entry.contains("version"))
          dep.version = entry["version"];
        else
          dep.version = "latest";
      }

      if (!dep.name.empty()) {
        deps.push_back(dep);
      }
    }
  }
  return deps;
}

} // namespace depdiscover