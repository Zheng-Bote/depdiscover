/**
 * SPDX-FileComment: Common Types and Structures
 * SPDX-FileType: SOURCE
 * SPDX-FileContributor: ZHENG Robert
 * SPDX-FileCopyrightText: 2026 ZHENG Robert
 * SPDX-License-Identifier: MIT
 *
 * @file types.hpp
 * @brief definitions of common data structures like Dependency and CVE.
 * @version 1.0.0
 * @date 2026-02-18
 *
 * @author ZHENG Robert (robert@hase-zheng.net)
 * @copyright Copyright (c) 2026 ZHENG Robert
 *
 * @license MIT License
 */
#pragma once
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

namespace depdiscover {

// CVE Struktur
/**
 * @brief Represents a Common Vulnerability and Exposure (CVE).
 */
struct CVE {
  std::string id;
  std::string summary;
  std::string severity;
  std::string fixed_version;
};

/**
 * @brief Serializes a CVE object to JSON.
 *
 * @param j The JSON object to write to.
 * @param c The CVE object.
 */
inline void to_json(nlohmann::json &j, const CVE &c) {
  j = nlohmann::json{{"id", c.id},
                     {"summary", c.summary},
                     {"severity", c.severity},
                     {"fixed_version", c.fixed_version}};
}

// Dependency Struktur (Erweitert)
/**
 * @brief Represents a software dependency.
 */
struct Dependency {
  std::string name;
  std::string version;
  std::string type;   // "vcpkg", "conan", "system"
  std::string source; // "manifest", "pkgconfig"

  std::vector<std::string> headers;
  std::vector<std::string> libraries;

  // NEU: Metadaten
  std::vector<std::string> licenses;
  std::vector<CVE> cves;
};

/**
 * @brief Serializes a Dependency object to JSON.
 *
 * @param j The JSON object to write to.
 * @param d The Dependency object.
 */
inline void to_json(nlohmann::json &j, const Dependency &d) {
  j = nlohmann::json{{"name", d.name},
                     {"version", d.version},
                     {"type", d.type},
                     {"source", d.source.empty() ? "manifest" : d.source},
                     {"headers", d.headers},
                     {"libraries", d.libraries},
                     {"licenses", d.licenses},
                     {"cves", d.cves}};
}

} // namespace depdiscover