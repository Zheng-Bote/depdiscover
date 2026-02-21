/**
 * SPDX-FileComment: HTML Report Generator
 * SPDX-FileType: SOURCE
 * SPDX-FileContributor: ZHENG Robert
 * SPDX-FileCopyrightText: 2026 ZHENG Robert
 * SPDX-License-Identifier: MIT
 *
 * @file html_generator.hpp
 * @brief Generates a user-friendly HTML report from the SBOM JSON data.
 * @version 1.2.0
 * @date 2026-02-21
 *
 * @author ZHENG Robert (robert@hase-zheng.net)
 * @copyright Copyright (c) 2026 ZHENG Robert
 *
 * @license MIT License
 */

#pragma once
#include <fstream>
#include <nlohmann/json.hpp>
#include <set>
#include <string>

namespace depdiscover {

inline void generate_html_report(const nlohmann::json &root,
                                 const std::string &filepath) {
  std::ofstream out(filepath);
  if (!out)
    return;

  // Basic HTML template with embedded CSS
  out << "<!DOCTYPE html>\n<html lang=\"de\">\n<head>\n"
      << "<meta charset=\"UTF-8\">\n"
      << "<meta name=\"viewport\" content=\"width=device-width, "
         "initial-scale=1.0\">\n"
      << "<title>SBOM Security Report</title>\n"
      << "<style>\n"
      << "  body { font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', "
         "Roboto, Helvetica, Arial, sans-serif; margin: 2rem; color: #333; }\n"
      << "  h1 { border-bottom: 2px solid #eee; padding-bottom: 0.5rem; }\n"
      << "  .metadata { background: #f8f9fa; padding: 1rem; border-radius: "
         "5px; margin-bottom: 2rem; }\n"
      << "  table { width: 100%; border-collapse: collapse; margin-top: 1rem; "
         "box-shadow: 0 1px 3px rgba(0,0,0,0.1); }\n"
      << "  th, td { text-align: left; padding: 12px 15px; border-bottom: 1px "
         "solid #ddd; vertical-align: top; }\n"
      << "  th { background-color: #343a40; color: white; }\n"
      << "  .safe { background-color: #d4edda; }\n"
      << "  .vuln { background-color: #f8d7da; }\n"
      << "  .warn { background-color: #fff3cd; }\n"
      << "  .vuln-title { font-weight: bold; color: #721c24; margin-bottom: "
         "5px; display: inline-block; }\n"
      << "  .badge { display: inline-block; padding: 0.25em 0.4em; font-size: "
         "75%; font-weight: 700; border-radius: 0.25rem; background: #6c757d; "
         "color: white; margin-right: 4px; }\n"
      << "  details { margin-top: 8px; font-size: 0.9em; }\n"
      << "  summary { cursor: pointer; font-weight: bold; color: #495057; }\n"
      << "  summary:hover { color: #000; }\n"
      << "  ul.clean-list { margin-top: 5px; padding-left: 20px; }\n" // Generische
                                                                      // Klasse
                                                                      // für
                                                                      // Listen
                                                                      // im
                                                                      // details
      << "  ul.clean-list li { margin-bottom: 4px; }\n"
      << "  a.cve-link { color: #0056b3; text-decoration: none; font-weight: "
         "bold; }\n"
      << "  a.cve-link:hover { text-decoration: underline; }\n"
      << "</style>\n</head>\n<body>\n";

  // Header Metadata
  if (root.contains("header")) {
    auto &h = root["header"];
    std::string proj = h.contains("project")
                           ? h["project"].value("name", "Unknown")
                           : "Unknown";
    std::string date = h.value("scan_date", "Unknown");
    out << "<h1>SBOM Security Report: " << proj << "</h1>\n";
    out << "<div class=\"metadata\">\n"
        << "  <strong>Scan Date:</strong> " << date << "<br>\n"
        << "  <strong>Generator Tool:</strong> "
        << h["tool"]["name"].get<std::string>() << " v"
        << h["tool"]["version"].get<std::string>() << "\n"
        << "</div>\n";
  }

  // Table Header
  out << "<table>\n"
      << "  <thead><tr>\n"
      << "    <th>Package Name</th>\n"
      << "    <th>Version</th>\n"
      << "    <th>Fixed Version</th>\n"
      << "    <th>Type</th>\n"
      << "    <th>Licenses</th>\n"
      << "    <th>Security Status</th>\n"
      << "  </tr></thead>\n"
      << "  <tbody>\n";

  // Dependencies
  if (root.contains("dependencies")) {
    for (const auto &dep : root["dependencies"]) {
      std::string name = dep.value("name", "Unknown");
      std::string version = dep.value("version", "Unknown");
      std::string type = dep.value("type", "Unknown");

      // Format Licenses
      std::string licenses = "";
      if (dep.contains("licenses")) {
        for (const auto &lic : dep["licenses"]) {
          licenses +=
              "<span class=\"badge\">" + lic.get<std::string>() + "</span>";
        }
      }

      // Security Status & Fixed Versions evaluieren
      std::string sec_class = "";
      std::string sec_text = "Unknown";
      std::set<std::string> unique_fixes; // Set für eindeutige Fix-Versionen

      if (dep.contains("cves") && !dep["cves"].empty()) {
        auto first_cve = dep["cves"][0];
        std::string first_id = first_cve.value("id", "UNKNOWN");

        if (first_id == "SAFE") {
          sec_class = "safe";
          sec_text = "✅ Safe";
        } else if (first_id == "NOT-CHECKED" || first_id == "CHECK-ERROR") {
          sec_class = "warn";
          sec_text = "⚠️ " + first_id;
        } else {
          sec_class = "vuln";
          size_t vuln_count = dep["cves"].size();

          // Roter Titel
          sec_text = "<span class=\"vuln-title\">❌ " +
                     std::to_string(vuln_count) + " Vulnerabilit" +
                     (vuln_count > 1 ? "ies" : "y") + "</span>";

          // Interaktives <details> Menü für CVEs
          sec_text += "\n<details><summary>Details anzeigen</summary>\n<ul "
                      "class=\"clean-list\">\n";

          for (const auto &cve : dep["cves"]) {
            std::string id = cve.value("id", "UNKNOWN");
            std::string summary = cve.value("summary", "");
            std::string fixed = cve.value("fixed_version", "");

            // Wenn es einen Fix gibt, in unser Set aufnehmen
            if (!fixed.empty()) {
              unique_fixes.insert(fixed);
            }

            // Dynamische Link-Generierung basierend auf der ID
            std::string url;
            if (id.find("CVE-") == 0) {
              url = "https://nvd.nist.gov/vuln/detail/" + id;
            } else if (id.find("GHSA-") == 0) {
              url = "https://github.com/advisories/" + id;
            } else {
              url = "https://osv.dev/vulnerability/" + id;
            }

            // Listen-Eintrag generieren
            sec_text += "  <li><a class=\"cve-link\" href=\"" + url +
                        "\" target=\"_blank\">" + id + "</a>";
            if (!summary.empty()) {
              if (summary.length() > 80)
                summary = summary.substr(0, 77) + "...";
              sec_text += ": " + summary;
            }
            sec_text += "</li>\n";
          }
          sec_text += "</ul>\n</details>";
        }
      }

      // NEU: Fixed Versions String als aufklappbares <details> Menü
      // zusammenbauen
      std::string fixed_versions_str = "-";
      if (!unique_fixes.empty()) {
        size_t fix_count = unique_fixes.size();
        std::string summary_text = std::to_string(fix_count) +
                                   (fix_count > 1 ? " Versionen" : " Version");

        fixed_versions_str = "<details><summary>" + summary_text +
                             "</summary><ul class=\"clean-list\">";
        for (const auto &f : unique_fixes) {
          fixed_versions_str += "<li><strong>v" + f + "</strong></li>";
        }
        fixed_versions_str += "</ul></details>";
      }

      // Write Row
      out << "    <tr class=\"" << sec_class << "\">\n"
          << "      <td><strong>" << name << "</strong></td>\n"
          << "      <td>" << version << "</td>\n"
          << "      <td>" << fixed_versions_str << "</td>\n"
          << "      <td>" << type << "</td>\n"
          << "      <td>" << licenses << "</td>\n"
          << "      <td>" << sec_text << "</td>\n"
          << "    </tr>\n";
    }
  }

  out << "  </tbody>\n</table>\n</body>\n</html>\n";
}

} // namespace depdiscover