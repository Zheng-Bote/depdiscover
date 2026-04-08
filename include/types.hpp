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
#include <algorithm>

namespace depdiscover {

/**
 * @brief Extracts a CVSS score from a severity string.
 *
 * @param severity_str The severity string (e.g., "7.5" or
 * "CVSS:3.1/AV:N/AC:L/PR:N/UI:N/S:U/C:H/I:H/A:H").
 * @return double The extracted CVSS score.
 */
inline double extract_cvss_score(const std::string &severity_str) {
  if (severity_str == "UNKNOWN" || severity_str == "NONE" ||
      severity_str.empty())
    return 0.0;

  try {
    return std::stod(severity_str);
  } catch (...) {
    if (severity_str.find("CVSS:") == 0) {
      int high_count = 0;
      if (severity_str.find("C:H") != std::string::npos)
        high_count++;
      if (severity_str.find("I:H") != std::string::npos)
        high_count++;
      if (severity_str.find("A:H") != std::string::npos)
        high_count++;

      if (high_count == 3)
        return 9.0; // Critical
      if (high_count > 0)
        return 7.0; // High
      return 5.0;   // Medium Fallback
    }
  }
  return 0.0;
}

/**
 * @brief Represents a Common Vulnerability and Exposure (CVE).
 */
struct CVE {
  std::string id;                  ///< The unique ID of the CVE (e.g., CVE-2021-1234).
  std::string summary;             ///< A brief summary of the vulnerability.
  std::string severity;            ///< The severity score (CVSS).
  double score = 0.0;              ///< The numeric CVSS score.
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
                     {"score", c.score},
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
