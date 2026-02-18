/**
 * SPDX-FileComment: License Resolver
 * SPDX-FileType: SOURCE
 * SPDX-FileContributor: ZHENG Robert
 * SPDX-FileCopyrightText: 2026 ZHENG Robert
 * SPDX-License-Identifier: MIT
 *
 * @file license_resolver.hpp
 * @brief Resolves licenses for dependencies using heuristics and file scanning.
 * @version 1.0.0
 * @date 2026-02-18
 *
 * @author ZHENG Robert (robert@hase-zheng.net)
 * @copyright Copyright (c) 2026 ZHENG Robert
 *
 * @license MIT License
 */
#pragma once
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <map>
#include <set>
#include <string>
#include <vector>

namespace depdiscover {

namespace fs = std::filesystem;

// --- Hilfsfunktionen für Text-Analyse ---

// Liest eine Datei und sucht nach Lizenz-Schlüsselwörtern
/**
 * @brief Guesses the license type from a file's content.
 *
 * Reads the first 20 lines and checks for common license keywords.
 *
 * @param path The path to the file (e.g., LICENSE).
 * @return std::string The matched license (e.g., "MIT"), or "See file: ..." if
 * ambiguous.
 */
inline std::string guess_license_from_content(const fs::path &path) {
  std::ifstream f(path);
  if (!f)
    return "";

  // Wir lesen nur die ersten 20 Zeilen (Performance & Relevanz)
  std::string content;
  std::string line;
  int lines_read = 0;
  while (std::getline(f, line) && lines_read < 20) {
    // ToUpper für einfachen Vergleich
    std::transform(line.begin(), line.end(), line.begin(), ::toupper);
    content += line + "\n";
    lines_read++;
  }

  if (content.find("MIT LICENSE") != std::string::npos)
    return "MIT";
  if (content.find("APACHE LICENSE") != std::string::npos &&
      content.find("VERSION 2.0") != std::string::npos)
    return "Apache-2.0";
  if (content.find("GNU GENERAL PUBLIC LICENSE") != std::string::npos) {
    if (content.find("VERSION 3") != std::string::npos)
      return "GPL-3.0";
    if (content.find("VERSION 2") != std::string::npos)
      return "GPL-2.0";
    return "GPL";
  }
  if (content.find("GNU LESSER GENERAL PUBLIC LICENSE") != std::string::npos)
    return "LGPL";
  if (content.find("BOOST SOFTWARE LICENSE") != std::string::npos)
    return "BSL-1.0";
  if (content.find("BSD 3-CLAUSE") != std::string::npos)
    return "BSD-3-Clause";
  if (content.find("BSD 2-CLAUSE") != std::string::npos)
    return "BSD-2-Clause";
  if (content.find("MOZILLA PUBLIC LICENSE") != std::string::npos)
    return "MPL-2.0";
  if (content.find("ZLIB LICENSE") != std::string::npos)
    return "Zlib";

  // Fallback: Wenn wir die Datei gefunden haben, aber den Text nicht erkennen,
  // geben wir "Custom" oder den Dateinamen zurück, damit der User nachsehen
  // kann.
  return "See file: " + path.filename().string();
}

// --- Hauptfunktion ---

/**
 * @brief Resolves licenses for a package.
 *
 * Uses a static database, name heuristics, and file system scanning of header
 * directories.
 *
 * @param package_name The name of the package.
 * @param header_files A list of associated header files (used for locating
 * license files).
 * @return std::vector<std::string> A list of resolved licenses.
 */
inline std::vector<std::string>
resolve_licenses(const std::string &package_name,
                 const std::vector<std::string> &header_files = {}) {
  std::vector<std::string> licenses;

  // 1. Statische Datenbank (Erweitert)
  static const std::map<std::string, std::vector<std::string>> license_db = {
      {"openssl", {"Apache-2.0"}},
      {"zlib", {"Zlib"}},
      {"fmt", {"MIT"}},
      {"nlohmann_json", {"MIT"}},
      {"spdlog", {"MIT"}},
      {"libcurl", {"curl"}},
      {"gtest", {"BSD-3-Clause"}},
      {"gmock", {"BSD-3-Clause"}},
      {"boost", {"BSL-1.0"}},
      {"protobuf", {"BSD-3-Clause"}},
      {"abseil", {"Apache-2.0"}},
      {"eigen", {"MPL-2.0"}},
      {"qt", {"LGPL-3.0", "GPL-2.0", "GPL-3.0"}},
      {"ffmpeg", {"LGPL-2.1"}}, // Oder GPL, je nach Config
      {"opencv", {"Apache-2.0"}},
      {"sqlite3", {"Public-Domain"}},
      {"catch2", {"BSL-1.0"}}};

  if (license_db.contains(package_name)) {
    return license_db.at(package_name);
  }

  // 2. Namens-Heuristik
  if (package_name.find("boost") != std::string::npos)
    return {"BSL-1.0"};
  if (package_name.find("gpl") != std::string::npos)
    return {"GPL"};

  // 3. File-System Scan (Intelligent)
  // Wir prüfen die Verzeichnisse, in denen die Header liegen.
  std::set<std::string> checked_dirs;

  for (const auto &header_path : header_files) {
    try {
      fs::path p(header_path);
      if (!p.is_absolute())
        continue;

      fs::path parent = p.parent_path();

      // Wir prüfen das direkte Verzeichnis und maximal 2 Ebenen darüber
      // (z.B. include/fmt/core.h -> include/fmt/LICENSE oder include/LICENSE)
      for (int i = 0; i < 3; ++i) {
        if (checked_dirs.contains(parent.string()))
          break;
        checked_dirs.insert(parent.string());

        // Typische Lizenz-Dateinamen
        std::vector<std::string> candidates = {
            "LICENSE",     "LICENSE.txt", "LICENSE.md",   "COPYING",
            "COPYING.txt", "NOTICE",      "Copyright.txt"};

        for (const auto &filename : candidates) {
          fs::path license_path = parent / filename;
          std::error_code ec;
          if (fs::exists(license_path, ec) &&
              fs::is_regular_file(license_path, ec)) {
            std::string detected = guess_license_from_content(license_path);
            if (!detected.empty()) {
              // Duplikate vermeiden
              bool already_have = false;
              for (const auto &l : licenses)
                if (l == detected)
                  already_have = true;
              if (!already_have)
                licenses.push_back(detected);
            }
          }
        }

        if (parent.has_parent_path())
          parent = parent.parent_path();
        else
          break;
      }
    } catch (...) {
      // Pfad-Fehler ignorieren
    }
  }

  if (licenses.empty()) {
    return {"UNKNOWN"};
  }

  return licenses;
}

} // namespace depdiscover