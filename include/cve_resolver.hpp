/**
 * SPDX-FileComment: CVE Resolver and OSV Query Tool
 * SPDX-FileType: SOURCE
 * SPDX-FileContributor: ZHENG Robert
 * SPDX-FileCopyrightText: 2026 ZHENG Robert
 * SPDX-License-Identifier: MIT
 *
 * @file cve_resolver.hpp
 * @brief Queries OSV.dev for CVEs associated with packages.
 * @version 1.3.1
 * @date 2026-02-18
 *
 * @author ZHENG Robert (robert@hase-zheng.net)
 * @copyright Copyright (c) 2026 ZHENG Robert
 *
 * @license MIT License
 */
#pragma once
#include "types.hpp"
#include <array>
#include <chrono>
#include <cstdio>
#include <filesystem>
#include <format>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <nlohmann/json.hpp>
#include <set>
#include <sstream>
#include <string>
#include <vector>

namespace depdiscover {

using json = nlohmann::json;
namespace fs = std::filesystem;

/**
 * @brief Returns the current date as a string in YYYY-MM-DD format.
 *
 * @return std::string The formatted date.
 */
inline std::string get_current_date() {
  auto now = std::chrono::system_clock::now();
  auto in_time_t = std::chrono::system_clock::to_time_t(now);
  std::stringstream ss;
  ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d");
  return ss.str();
}

/**
 * @brief Performs a secure curl POST request using a temporary file for the payload.
 *
 * @param url The target URL.
 * @param json_payload The JSON payload to send.
 * @return std::string The response from the server.
 */
inline std::string perform_curl_post(const std::string &url,
                                     const std::string &json_payload) {
  std::string response;

  fs::path tmp_file = fs::temp_directory_path() / "depdiscover_osv_query.json";
  {
    std::ofstream out(tmp_file);
    out << json_payload;
  }

  std::string cmd = std::format("curl -s -L -X POST -H \"Content-Type: "
                                "application/json\" -d @\"{}\" \"{}\"",
                                tmp_file.string(), url);

  std::array<char, 1024> buffer;
  std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "r"),
                                                pclose);

  if (!pipe) {
    std::cerr << "[Error] Could not run curl command.\n";
    return "";
  }

  while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
    response += buffer.data();
  }

  std::error_code ec;
  fs::remove(tmp_file, ec);

  return response;
}

/**
 * @brief Queries OSV.dev for vulnerabilities related to a package and version.
 *
 * @param name The name of the package.
 * @param version The version of the package.
 * @param ecosystem The OSV ecosystem (default: "Debian").
 * @return std::vector<CVE> A list of vulnerabilities found.
 */
inline std::vector<CVE> query_cves(const std::string &name,
                                   const std::string &version,
                                   const std::string &ecosystem = "Debian") {
  std::vector<CVE> results;

  if (name.empty() || version.empty() || version == "unknown" ||
      version == "latest") {
    results.push_back({"NOT-CHECKED",
                       "Version unknown or latest, cannot query OSV", "UNKNOWN",
                       "", false, ""});
    return results;
  }

  std::string actual_name = (name == "libcurl") ? "curl" : name;

  json query;
  query["package"] = {{"name", actual_name}, {"ecosystem", ecosystem}};
  query["version"] = version;

  std::cerr << "   [CVE Check] " << actual_name << " @ " << version << " ("
            << ecosystem << ") ... ";

  std::string json_str = query.dump();
  std::string response =
      perform_curl_post("https://api.osv.dev/v1/query", json_str);

  if (response.empty()) {
    std::cerr << "Failed (Network Error)\n";
    results.push_back(
        {"CHECK-ERROR", "Network request failed or no output from curl",
         "UNKNOWN", "", false, ""});
    return results;
  }

  try {
    auto doc = json::parse(response);

    if (doc.contains("message") && doc.contains("code")) {
      std::string error_msg = doc["message"].get<std::string>();
      std::cerr << "API Error (" << error_msg << ")\n";
      results.push_back({"CHECK-ERROR", "OSV API Error: " + error_msg,
                         "UNKNOWN", "", false,
                         ""});
      return results;
    }

    if (doc.contains("vulns") && doc["vulns"].is_array()) {
      std::cerr << "FOUND " << doc["vulns"].size() << " Vulns!\n";

      for (const auto &item : doc["vulns"]) {
        CVE cve;

        // ID extraction (Try to get real CVE from aliases if it's a DEBIAN internal ID)
        std::string id = item.value("id", "UNKNOWN");
        if (id.starts_with("DEBIAN-CVE") && item.contains("aliases") &&
            item["aliases"].is_array() && !item["aliases"].empty()) {
          id = item["aliases"][0].get<std::string>();
        }
        cve.id = id;

        // Summary extraction (fallback to 'details' as Debian often lacks 'summary')
        cve.summary = item.value("summary", "");
        if (cve.summary.empty() && item.contains("details")) {
          std::string details = item.value("details", "");
          // Truncate long details for overview
          if (details.length() > 150)
            details = details.substr(0, 147) + "...";

          // Remove newlines for cleaner JSON
          std::replace(details.begin(), details.end(), '\n', ' ');
          cve.summary = details;
        }
        if (cve.summary.empty())
          cve.summary = "No summary available";

        // Severity (Debian often lacks severity, use fallback)
        if (item.contains("severity") && item["severity"].is_array() &&
            !item["severity"].empty()) {
          cve.severity = item["severity"][0].value("score", "UNKNOWN");
        } else {
          cve.severity = "UNKNOWN";
        }

        // Fixed Version
        if (item.contains("affected") && item["affected"].is_array()) {
          for (const auto &affected : item["affected"]) {
            if (affected.contains("ranges")) {
              for (const auto &range : affected["ranges"]) {
                if (range.contains("events")) {
                  for (const auto &event : range["events"]) {
                    if (event.contains("fixed")) {
                      cve.fixed_version = event["fixed"];
                      break;
                    }
                  }
                }
              }
            }
          }
        }

        cve.suppressed = false;
        cve.suppression_reason = "";

        results.push_back(cve);
      }
    } else {
      std::cerr << "OK (Safe)\n";
      CVE safe_entry;
      safe_entry.id = "SAFE";
      safe_entry.severity = "NONE";
      safe_entry.fixed_version = "";
      safe_entry.summary = "No vulnerabilities found in ecosystem '" +
                           ecosystem + "'. Checked on " + get_current_date();
      safe_entry.suppressed = false;
      safe_entry.suppression_reason = "";
      results.push_back(safe_entry);
    }
  } catch (const std::exception &e) {
    std::cerr << "JSON Error: " << e.what() << "\n";
    results.push_back({"CHECK-ERROR",
                       std::string("JSON parse error: ") + e.what(), "UNKNOWN",
                       "", false, ""});
  }

  return results;
}

} // namespace depdiscover
