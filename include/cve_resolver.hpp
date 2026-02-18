#pragma once
#include "types.hpp" // Für struct CVE
#include <array>
#include <chrono> // NEU: Für Datum
#include <cstdio>
#include <format>
#include <iomanip> // NEU: Für Datum Formatierung
#include <iostream>
#include <memory>
#include <nlohmann/json.hpp>
#include <set>
#include <sstream>
#include <string>
#include <vector>

namespace depdiscover {

using json = nlohmann::json;

// --- Hilfsfunktion: Datum holen ---
inline std::string get_current_date() {
  auto now = std::chrono::system_clock::now();
  auto in_time_t = std::chrono::system_clock::to_time_t(now);

  std::stringstream ss;
  // Format: YYYY-MM-DD
  ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d");
  return ss.str();
}

// --- Strategie-Auswahl für OSV ---

struct QueryStrategy {
  std::string name;
  std::string param; // Ecosystem oder PURL-Type
  bool use_purl;
};

inline QueryStrategy determine_strategy(const std::string &pkg_name) {
  static const std::set<std::string> oss_fuzz_projects = {
      "openssl",     "zlib",     "curl",    "libcurl",   "ffmpeg",
      "sqlite3",     "pcre",     "pcre2",   "libpng",    "libjpeg-turbo",
      "freetype2",   "harfbuzz", "systemd", "wireshark", "tcpdump",
      "imagemagick", "poppler",  "expat"};

  if (pkg_name == "libcurl")
    return {"curl", "OSS-Fuzz", false};

  if (oss_fuzz_projects.contains(pkg_name)) {
    return {pkg_name, "OSS-Fuzz", false};
  }

  return {pkg_name, "generic", true};
}

// --- Curl Wrapper ---

inline std::string perform_curl_post(const std::string &url,
                                     const std::string &json_payload) {
  std::string response;

  // Escaping
  std::string escaped_json = json_payload;
  size_t pos = 0;
  while ((pos = escaped_json.find("'", pos)) != std::string::npos) {
    escaped_json.replace(pos, 1, "'\\''");
    pos += 4;
  }

  std::string cmd =
      std::format("echo '{}' | curl -s -L -X POST -H \"Content-Type: "
                  "application/json\" -d @- \"{}\"",
                  escaped_json, url);

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

  return response;
}

// --- Hauptfunktion ---

inline std::vector<CVE> query_cves(const std::string &name,
                                   const std::string &version) {
  std::vector<CVE> results;

  // Checks: Keine Abfrage ohne Version
  if (name.empty() || version.empty() || version == "unknown" ||
      version == "latest") {
    // Optional: Eintrag, dass nicht geprüft werden konnte
    results.push_back({"NOT-CHECKED",
                       "Version unknown or latest, cannot query OSV", "UNKNOWN",
                       ""});
    return results;
  }

  QueryStrategy strat = determine_strategy(name);

  json query;
  std::string check_info; // Für den Log/Summary

  if (strat.use_purl) {
    std::string purl =
        std::format("pkg:{}/{}@{}", strat.param, strat.name, version);
    query["purl"] = purl;
    check_info = "PURL: " + purl;
    std::cerr << "   [CVE Check] " << check_info << " ... ";
  } else {
    query["package"] = {{"name", strat.name}, {"ecosystem", strat.param}};
    query["version"] = version;
    check_info = "Ecosystem: " + strat.param;
    std::cerr << "   [CVE Check] " << strat.name << " @ " << version << " ("
              << strat.param << ") ... ";
  }

  std::string json_str = query.dump();
  std::string response =
      perform_curl_post("https://api.osv.dev/v1/query", json_str);

  if (response.empty()) {
    std::cerr << "Failed (Network Error)\n";
    results.push_back({"CHECK-ERROR",
                       "Network request failed or no output from curl",
                       "UNKNOWN", ""});
    return results;
  }

  try {
    auto doc = json::parse(response);

    if (doc.contains("vulns") && doc["vulns"].is_array()) {
      size_t count = doc["vulns"].size();
      std::cerr << "FOUND " << count << " Vulns!\n";

      for (const auto &item : doc["vulns"]) {
        CVE cve;
        cve.id = item.value("id", "UNKNOWN");
        cve.summary = item.value("summary", "No summary available");

        if (item.contains("severity") && item["severity"].is_array() &&
            !item["severity"].empty()) {
          cve.severity = item["severity"][0].value("score", "UNKNOWN");
        } else {
          cve.severity = "UNKNOWN";
        }

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
        results.push_back(cve);
      }
    } else {
      // KEINE SCHWACHSTELLEN GEFUNDEN -> SAFE EINTRAG
      std::cerr << "OK (Safe)\n";

      CVE safe_entry;
      safe_entry.id = "SAFE"; // Signalwort für UI/Parser
      safe_entry.severity = "NONE";
      safe_entry.fixed_version = "";
      // Hier speichern wir das Datum der Prüfung
      safe_entry.summary = "No known vulnerabilities found. Checked on " +
                           get_current_date() + " via OSV.dev (" + check_info +
                           ")";

      results.push_back(safe_entry);
    }
  } catch (const std::exception &e) {
    std::cerr << "JSON Error: " << e.what() << "\n";
    results.push_back({"CHECK-ERROR",
                       std::string("JSON parse error: ") + e.what(), "UNKNOWN",
                       ""});
  }

  return results;
}

} // namespace depdiscover