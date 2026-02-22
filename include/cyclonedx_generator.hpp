/**
 * SPDX-FileComment: CycloneDX Generator
 * SPDX-FileType: SOURCE
 * SPDX-FileContributor: ZHENG Robert
 * SPDX-FileCopyrightText: 2026 ZHENG Robert
 * SPDX-License-Identifier: MIT
 *
 * @file cyclonedx_generator.hpp
 * @brief Generates a valid CycloneDX 1.4 SBOM from internal JSON data.
 * @version 1.0.0
 * @date 2026-02-22
 *
 * @author ZHENG Robert (robert@hase-zheng.net)
 * @copyright Copyright (c) 2026 ZHENG Robert
 *
 * @license MIT License
 */

#pragma once
#include <chrono>
#include <fstream>
#include <iomanip>
#include <nlohmann/json.hpp>
#include <random>
#include <sstream>
#include <string>

namespace depdiscover {

using json = nlohmann::json;

// Hilfsfunktion: ISO-8601 Timestamp für CycloneDX (YYYY-MM-DDThh:mm:ssZ)
inline std::string get_iso8601_timestamp() {
  auto now = std::chrono::system_clock::now();
  auto in_time_t = std::chrono::system_clock::to_time_t(now);
  std::stringstream ss;
  ss << std::put_time(std::gmtime(&in_time_t), "%Y-%m-%dT%H:%M:%SZ");
  return ss.str();
}

// Hilfsfunktion: Pseudo-UUIDv4 für die SBOM Serial Number
inline std::string generate_uuid_v4() {
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<> dis(0, 15);
  std::uniform_int_distribution<> dis2(8, 11);

  const char *hex_chars = "0123456789abcdef";
  std::string uuid = "00000000-0000-4000-8000-000000000000";

  for (int i = 0; i < 36; ++i) {
    if (uuid[i] == '0') {
      uuid[i] = hex_chars[dis(gen)];
    } else if (uuid[i] == '8') {
      uuid[i] = hex_chars[dis2(gen)];
    }
  }
  return uuid;
}

inline void generate_cyclonedx_report(const json &internal_root,
                                      const std::string &filepath) {
  json cdx;
  cdx["bomFormat"] = "CycloneDX";
  cdx["specVersion"] = "1.4";
  cdx["serialNumber"] = "urn:uuid:" + generate_uuid_v4();
  cdx["version"] = 1;

  // --- Metadata ---
  json metadata;
  metadata["timestamp"] = get_iso8601_timestamp();

  if (internal_root.contains("header")) {
    auto &h = internal_root["header"];

    // Tool Info
    json tool;
    tool["vendor"] = h["tool"].value("author", "Unknown");
    tool["name"] = h["tool"].value("name", "depdiscover");
    tool["version"] = h["tool"].value("version", "1.0.0");
    metadata["tools"] = json::array({tool});

    // Project/Component Info
    json component;
    component["type"] = "application";
    component["name"] = h["project"].value("name", "UnknownProject");
    metadata["component"] = component;
  }
  cdx["metadata"] = metadata;

  // --- Components & Vulnerabilities ---
  json components = json::array();
  json vulnerabilities = json::array();

  if (internal_root.contains("dependencies")) {
    for (const auto &dep : internal_root["dependencies"]) {
      json comp;
      std::string name = dep.value("name", "unknown");
      std::string version = dep.value("version", "unknown");

      // PURL (Package URL) als eindeutige Referenz
      std::string purl = "pkg:generic/" + name + "@" + version;

      comp["type"] = "library";
      comp["name"] = name;
      comp["version"] = version;
      comp["bom-ref"] = purl; // Verknüpfungspunkt für CVEs
      comp["purl"] = purl;

      // Licenses
      if (dep.contains("licenses") && !dep["licenses"].empty()) {
        json licenses_arr = json::array();
        for (const auto &lic : dep["licenses"]) {
          std::string l_str = lic.get<std::string>();
          // Wir nehmen vereinfacht an, dass unsere Lizenzen SPDX-IDs sind
          // Fallback wäre {"license": {"name": l_str}}
          if (l_str != "UNKNOWN" && l_str != "unknown") {
            licenses_arr.push_back({{"license", {{"id", l_str}}}});
          } else {
            licenses_arr.push_back({{"license", {{"name", l_str}}}});
          }
        }
        comp["licenses"] = licenses_arr;
      }

      components.push_back(comp);

      // Vulnerabilities
      if (dep.contains("cves") && !dep["cves"].empty()) {
        for (const auto &cve : dep["cves"]) {
          std::string id = cve.value("id", "UNKNOWN");

          // Wir ignorieren unsere internen "Safe"-Marker für CycloneDX
          if (id == "SAFE" || id == "NOT-CHECKED" || id == "CHECK-ERROR") {
            continue;
          }

          json vuln;
          vuln["bom-ref"] = purl; // Zeigt auf die Component oben
          vuln["id"] = id;

          json source;
          source["name"] = "OSV.dev";
          vuln["source"] = source;

          std::string summary = cve.value("summary", "");
          if (!summary.empty())
            vuln["description"] = summary;

          // Dummy Rating, da OSV den Score als String liefert ("UNKNOWN" oder
          // "7.5" etc) In einer echten Umsetzung müsste man den CVSS String
          // parsen
          std::string severity = cve.value("severity", "UNKNOWN");
          if (severity != "UNKNOWN" && severity != "NONE") {
            json rating;
            rating["severity"] = "unknown"; // CycloneDX erfordert enum: low,
                                            // medium, high, critical
            // Optional: String in double umwandeln für rating["score"]
            vuln["ratings"] = json::array({rating});
          }

          // Advisories / Links
          json advisory;
          if (id.find("CVE-") == 0)
            advisory["url"] = "https://nvd.nist.gov/vuln/detail/" + id;
          else if (id.find("GHSA-") == 0)
            advisory["url"] = "https://github.com/advisories/" + id;
          else
            advisory["url"] = "https://osv.dev/vulnerability/" + id;
          vuln["advisories"] = json::array({advisory});

          vulnerabilities.push_back(vuln);
        }
      }
    }
  }

  cdx["components"] = components;
  if (!vulnerabilities.empty()) {
    cdx["vulnerabilities"] = vulnerabilities;
  }

  // Speichern
  std::ofstream out(filepath);
  if (out) {
    out << cdx.dump(2);
  }
}

} // namespace depdiscover