/**
 * SPDX-FileComment: Common Types and Structures
 * SPDX-FileType: SOURCE
 * SPDX-FileContributor: ZHENG Robert
 * SPDX-FileCopyrightText: 2026 ZHENG Robert
 * SPDX-License-Identifier: MIT
 *
 * @file types.hpp
 * @brief definitions of common data structures like Dependency and CVE.
 * @version 1.1.0
 * @date 2026-02-22
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

/**
 * @brief Represents a Common Vulnerability and Exposure (CVE).
 */
struct CVE {
  std::string id;                  ///< The unique ID of the CVE (e.g., CVE-2021-1234).
  std::string summary;             ///< A brief summary of the vulnerability.
  std::string severity;            ///< The severity score (CVSS).
  std::string fixed_version;       ///< The version where the vulnerability is fixed.
  bool suppressed = false;         ///< True if this vulnerability is manually suppressed.
  std::string suppression_reason;  ///< The reason for suppression.
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
                     {"fixed_version", c.fixed_version},
                     {"suppressed", c.suppressed},
                     {"suppression_reason", c.suppression_reason}};
}

/**
 * @brief Represents a software dependency.
 */
struct Dependency {
  std::string name;                ///< Package name.
  std::string version;             ///< Package version.
  std::string type;                ///< Dependency type (e.g., "vcpkg", "conan", "system").
  std::string source;              ///< Metadata source (e.g., "manifest", "pkgconfig").

  std::vector<std::string> headers;   ///< List of associated header files.
  std::vector<std::string> libraries; ///< List of associated binary libraries.

  std::vector<std::string> licenses;  ///< List of resolved licenses.
  std::vector<CVE> cves;              ///< List of identified vulnerabilities.
};

/**
 * @brief Serializes a Dependency object to JSON.
 *
 * @param j The JSON object to write to.
 * @param d The Dependency object.
 */
inline void to_json(nlohmann::json &j, const Dependency &d) {
  j = nlohmann::json{
      {"name", d.name},
      {"version", d.version},
      {"type", d.type},
      {"source", d.source.empty() ? "manifest" : d.source},
      {"headers", d.headers},
      {"libraries", d.libraries},
      {"licenses", d.licenses},
      {"cves", d.cves}};
}

} // namespace depdiscover
