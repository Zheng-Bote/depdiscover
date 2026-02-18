/**
 * SPDX-FileComment: Main entry point for the Dependency Tracker application
 * SPDX-FileType: SOURCE
 * SPDX-FileContributor: ZHENG Robert
 * SPDX-FileCopyrightText: 2026 ZHENG Robert
 * SPDX-License-Identifier: MIT
 *
 * @file main.cpp
 * @brief Main entry point for the Dependency Tracker application.
 * @version 1.0.0
 * @date 2026-02-18
 *
 * @author ZHENG Robert (robert@hase-zheng.net)
 * @copyright Copyright (c) 2026 ZHENG Robert
 *
 * @license MIT License
 */

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <set>
#include <sstream>
#include <string>
#include <vector>

// Externe Bibliotheken
#include <nlohmann/json.hpp>

// Projekt Konfiguration (NEU)
#include "rz_config.hpp"

// Core Komponenten
#include "compile_commands.hpp"
#include "elf_scanner.hpp"
#include "header_resolver.hpp"
#include "include_scanner.hpp"
#include "pkg_config.hpp"
#include "semver.hpp"
#include "types.hpp"

// Parser
#include "cmake_libs_parser.hpp"
#include "conan_parser.hpp"
#include "vcpkg_parser.hpp"

// Metadaten Resolver
#include "cve_resolver.hpp"
#include "license_resolver.hpp"

using namespace depdiscover;
using json = nlohmann::json;
namespace fs = std::filesystem;

// --- Konstanten via rz_config.hpp ---
// Wir nutzen jetzt direkt rz::config::... unten im Code.
constexpr auto SCHEMA_VERSION = "1.2";

// --- Hilfsfunktionen für Matching ---

/**
 * @brief Checks if a string contains another string (case-insensitive).
 *
 * @param haystack The string to search in.
 * @param needle The substring to search for.
 * @return true if haystack contains needle, false otherwise.
 */
bool string_contains(const std::string &haystack, const std::string &needle) {
  auto it = std::search(haystack.begin(), haystack.end(), needle.begin(),
                        needle.end(), [](char ch1, char ch2) {
                          return std::tolower(ch1) == std::tolower(ch2);
                        });
  return it != haystack.end();
}

/**
 * @brief Checks if a path starts with a given prefix.
 *
 * @param path The path to check.
 * @param prefix The prefix to look for.
 * @return true if path starts with prefix, false otherwise.
 */
bool path_starts_with(const std::string &path, const std::string &prefix) {
  if (path.rfind(prefix, 0) == 0)
    return true;
  if (!prefix.empty() && prefix.back() != '/' &&
      path.rfind(prefix + "/", 0) == 0)
    return true;
  return false;
}

/**
 * @brief Fuzzy matches a header path against a package name.
 *
 * @param header_path The path to the header file.
 * @param pkg_name The name of the package.
 * @return true if the header path likely belongs to the package.
 */
bool fuzzy_match_header(const std::string &header_path,
                        const std::string &pkg_name) {
  if (string_contains(header_path, "/" + pkg_name + "/"))
    return true;
  if (string_contains(header_path, "/" + pkg_name + ".h"))
    return true;
  if (string_contains(header_path, "/" + pkg_name + ".hpp"))
    return true;

  std::string clean_name = pkg_name;
  std::replace(clean_name.begin(), clean_name.end(), '_', ' ');
  std::replace(clean_name.begin(), clean_name.end(), '-', ' ');

  std::stringstream ss(clean_name);
  std::string segment;
  while (ss >> segment) {
    if (segment.length() < 3)
      continue;
    if (segment == "lib")
      continue;
    if (string_contains(header_path, "/" + segment + "/"))
      return true;
  }
  return false;
}

/**
 * @brief Fuzzy matches a library filename against a package name.
 *
 * @param lib_filename The filename of the library.
 * @param pkg_name The name of the package.
 * @return true if the library filename likely belongs to the package.
 */
bool fuzzy_match_lib(const std::string &lib_filename,
                     const std::string &pkg_name) {
  std::string clean_filename = lib_filename;
  if (clean_filename.rfind("lib", 0) == 0)
    clean_filename = clean_filename.substr(3);
  std::string clean_pkg = pkg_name;
  if (clean_pkg.rfind("lib", 0) == 0)
    clean_pkg = clean_pkg.substr(3);
  return (clean_filename.rfind(clean_pkg, 0) == 0);
}

/**
 * @brief Prints the help message to stderr.
 *
 * @param program_name The name of the program (argv[0]).
 */
void print_help(const char *program_name) {
  std::cerr
      << "Verwendung: " << program_name << " [OPTIONEN]\n\n"
      << "Optionen:\n"
      << "  -c, --compile-commands <PFAD>  Input: compile_commands.json\n"
      << "  -l, --libs <PFAD>              Input: libs.txt (CMake generated)\n"
      << "  -b, --binary <PFAD>            Input: Binary (für ldd Analyse)\n"
      << "  -v, --vcpkg <PFAD>             Input: vcpkg.json\n"
      << "  -C, --conan <PFAD>             Input: conanfile.txt\n"
      << "  -o, --output <PFAD>            Output: JSON Datei (Default: "
         "depdiscover.json)\n"
      << "  -n, --name <NAME>              Setzt den Projektnamen im Report\n"
      << "  -h, --help                     Zeigt diese Hilfe an\n\n"
      << "Info:\n"
      << "  " << rz::config::PROG_LONGNAME << "\n"
      << "  Version: " << rz::config::VERSION << "\n"
      << "  Author:  " << rz::config::AUTHOR << "\n";
}

// --- Main ---

/**
 * @brief Main function of the application.
 *
 * Parses command line arguments, loads dependencies from various sources
 * (vcpkg, conan, CMake), scans build artifacts (compile_commands.json,
 * binaries), resolves metadata (licenses, CVEs), and generates a JSON report.
 *
 * @param argc Number of arguments.
 * @param argv Argument strings.
 * @return int Exit code (0 for success, 1 for error).
 */
int main(int argc, char **argv) {
  // Standardpfade & Werte
  std::string cc_path = "compile_commands.json";
  std::string libs_txt_path = "libs.txt";
  std::string binary_path = "";
  std::string vcpkg_path = "vcpkg.json";
  std::string conan_path = "conanfile.txt";
  std::string output_path = "depdiscover.json";
  std::string project_name = "Unknown Project";

  // Argument Parsing
  for (int i = 1; i < argc; ++i) {
    std::string arg = argv[i];
    if (arg == "-h" || arg == "--help") {
      print_help(argv[0]);
      return 0;
    } else if (arg == "-c" || arg == "--compile-commands") {
      if (i + 1 < argc)
        cc_path = argv[++i];
    } else if (arg == "-l" || arg == "--libs") {
      if (i + 1 < argc)
        libs_txt_path = argv[++i];
    } else if (arg == "-b" || arg == "--binary") {
      if (i + 1 < argc)
        binary_path = argv[++i];
    } else if (arg == "-v" || arg == "--vcpkg") {
      if (i + 1 < argc)
        vcpkg_path = argv[++i];
    } else if (arg == "-C" || arg == "--conan") {
      if (i + 1 < argc)
        conan_path = argv[++i];
    } else if (arg == "-o" || arg == "--output") {
      if (i + 1 < argc)
        output_path = argv[++i];
    } else if (arg == "-n" || arg == "--name") {
      if (i + 1 < argc)
        project_name = argv[++i];
    }
  }

  try {
    // --- 1. Abhängigkeiten laden ---
    std::vector<Dependency> deps;

    if (fs::exists(vcpkg_path)) {
      std::cerr << "[Info] Lade Vcpkg Manifest: " << vcpkg_path << "\n";
      auto v = parse_vcpkg_manifest(vcpkg_path);
      deps.insert(deps.end(), v.begin(), v.end());
    }
    if (fs::exists(conan_path)) {
      std::cerr << "[Info] Lade Conan File: " << conan_path << "\n";
      auto c = parse_conan_dependencies(conan_path);
      deps.insert(deps.end(), c.begin(), c.end());
    }
    if (fs::exists(libs_txt_path)) {
      std::cerr << "[Info] Lade CMake libs.txt: " << libs_txt_path << "\n";
      auto cmake_deps = parse_cmake_libs(libs_txt_path);
      for (const auto &cd : cmake_deps) {
        bool found = false;
        for (auto &existing : deps) {
          if (existing.name == cd.name ||
              cd.name.find(existing.name) != std::string::npos ||
              existing.name.find(cd.name) != std::string::npos) {
            found = true;
            if (existing.version == "latest" || existing.version == "unknown") {
              if (cd.version != "unknown")
                existing.version = cd.version;
            }
            break;
          }
        }
        if (!found)
          deps.push_back(cd);
      }
    }

    // --- 2. Build-Artefakte scannen ---
    std::set<std::string> all_resolved_headers;
    std::set<std::string> all_elf_libs;

    std::cerr << "[Info] Analysiere Compile Commands...\n";
    auto cc = load_compile_commands(cc_path);
    for (const auto &entry : cc) {
      auto incs = extract_include_paths(entry.command);
      std::vector<std::string> inc_vec(incs.begin(), incs.end());
      auto raw = scan_includes(entry.file);
      for (const auto &r : raw) {
        std::string path = resolve_header(r, inc_vec, entry.directory);
        if (!path.empty())
          all_resolved_headers.insert(path);
      }
    }
    std::cerr << "   -> " << all_resolved_headers.size()
              << " Header-Dateien identifiziert.\n";

    if (!binary_path.empty()) {
      std::cerr << "[Info] Scanne Binary (ELF): " << binary_path << "\n";
      auto l = scan_elf_dependencies(binary_path);
      all_elf_libs.insert(l.begin(), l.end());
    }

    // --- 3. Mapping & Anreicherung ---
    std::cerr << "[Info] Starte Mapping & Metadaten-Abfrage...\n";

    for (auto &dep : deps) {
      PkgInfo pkg = PkgConfig::query(dep.name);
      if (pkg.found) {
        dep.version = pkg.version;
        dep.source = "pkgconfig";
        for (const auto &dir : pkg.include_paths) {
          for (auto it = all_resolved_headers.begin();
               it != all_resolved_headers.end();) {
            if (path_starts_with(*it, dir)) {
              dep.headers.push_back(*it);
              it = all_resolved_headers.erase(it);
            } else
              ++it;
          }
        }
        for (const auto &l_name : pkg.lib_names) {
          for (auto it = all_elf_libs.begin(); it != all_elf_libs.end();) {
            if (it->find("lib" + l_name) != std::string::npos) {
              dep.libraries.push_back(*it);
              it = all_elf_libs.erase(it);
            } else
              ++it;
          }
        }
      }

      if (dep.headers.empty()) {
        for (auto it = all_resolved_headers.begin();
             it != all_resolved_headers.end();) {
          if (fuzzy_match_header(*it, dep.name)) {
            dep.headers.push_back(*it);
            it = all_resolved_headers.erase(it);
          } else
            ++it;
        }
      }
      if (dep.headers.empty()) {
        for (auto it = all_resolved_headers.begin();
             it != all_resolved_headers.end();) {
          if (string_contains(*it, dep.name)) {
            dep.headers.push_back(*it);
            it = all_resolved_headers.erase(it);
          } else
            ++it;
        }
      }
      if (dep.libraries.empty()) {
        for (auto it = all_elf_libs.begin(); it != all_elf_libs.end();) {
          if (fuzzy_match_lib(*it, dep.name)) {
            dep.libraries.push_back(*it);
            it = all_elf_libs.erase(it);
          } else
            ++it;
        }
      }

      dep.licenses = resolve_licenses(dep.name, dep.headers);

      std::string clean_ver = dep.version;
      if (!clean_ver.empty() && clean_ver[0] == 'v')
        clean_ver.erase(0, 1);
      dep.cves = query_cves(dep.name, clean_ver);
    }

    // --- 4. System Libs ---
    for (const auto &lib : all_elf_libs) {
      Dependency sys;
      sys.name = lib;
      sys.type = "system";
      sys.source = "elf_scan";
      sys.libraries.push_back(lib);
      sys.licenses = resolve_licenses(lib);
      deps.push_back(sys);
    }

    // --- 5. Output Generieren (Erweiterter Header) ---
    json root;

    // Header Block
    json header;
    header["schema_version"] = SCHEMA_VERSION;
    header["scan_date"] = get_current_date();

    // Tool Metadata via rz_config.hpp
    json tool;
    tool["name"] = rz::config::PROJECT_NAME; // "depdiscover"
    tool["version"] = rz::config::VERSION;   // "1.0.0"
    tool["description"] =
        rz::config::PROG_LONGNAME; // "Native C++ Dependency Scanner..."
    tool["homepage"] = rz::config::PROJECT_HOMEPAGE_URL;
    tool["author"] = rz::config::AUTHOR;
    header["tool"] = tool;

    // Project Metadata
    json project;
    project["name"] = project_name;
    project["workspace_root"] = fs::current_path().string();
    header["project"] = project;

    root["header"] = header;
    root["dependencies"] = deps;

    std::ofstream out_file(output_path);
    if (!out_file) {
      std::cerr << "Error: Konnte Ausgabedatei nicht schreiben: " << output_path
                << "\n";
      return 1;
    }

    out_file << root.dump(2);
    std::cerr << "[Success] SBOM Report geschrieben nach: " << output_path
              << "\n";

  } catch (const std::exception &ex) {
    std::cerr << "Error: " << ex.what() << "\n";
    return 1;
  }
  return 0;
}