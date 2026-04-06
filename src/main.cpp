/**
 * SPDX-FileComment: Main entry point for the Dependency Tracker application
 * SPDX-FileType: SOURCE
 * SPDX-FileContributor: ZHENG Robert
 * SPDX-FileCopyrightText: 2026 ZHENG Robert
 * SPDX-License-Identifier: MIT
 *
 * @file main.cpp
 * @brief Main entry point for the Dependency Tracker application.
 * @version 1.3.0
 * @date 2026-04-06

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

// External Libraries
#include <nlohmann/json.hpp>

// Project Configuration
#include "rz_config.hpp"

// Update Checker
#include <check_gh-update.hpp>
#include <print>

// Core Components
#include "compile_commands.hpp"
#include "elf_scanner.hpp"
#include "header_resolver.hpp"
#include "include_scanner.hpp"
#include "pkg_config.hpp"
#include "semver.hpp"
#include "types.hpp"

// Parsers
#include "cmake_fetch_parser.hpp"
#include "cmake_libs_parser.hpp"
#include "conan_parser.hpp"
#include "vcpkg_parser.hpp"

// Metadata & Output Resolvers
#include "cve_resolver.hpp"
#include "cyclonedx_generator.hpp"
#include "html_generator.hpp"
#include "license_resolver.hpp"

using namespace depdiscover;
using json = nlohmann::json;
namespace fs = std::filesystem;

/**
 * @brief Current schema version for the generated JSON report.
 */
constexpr auto SCHEMA_VERSION = "1.2";

/**
 * @brief Checks if a string contains another string (case-insensitive).
 *
 * @param haystack The string to search in.
 * @param needle The string to search for.
 * @return true If the needle is found in the haystack.
 * @return false Otherwise.
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
 * @return true If the path starts with the prefix.
 * @return false Otherwise.
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
 * @brief Identifies the current OS platform.
 *
 * @return std::string "Windows", "macOS", "Linux" or "Unknown".
 */
std::string get_platform_name() {
#if defined(_WIN32) || defined(_WIN64)
  return "Windows";
#elif defined(__APPLE__) || defined(__MACH__)
  return "macOS";
#elif defined(__linux__)
  return "Linux";
#else
  return "Unknown";
#endif
}

/**
 * @brief Performs a fuzzy match between a header path and a package name.
 *
 * @param header_path The path to the header file.
 * @param pkg_name The name of the package.
 * @return true If a match is likely.
 * @return false Otherwise.
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
 * @brief Performs a fuzzy match between a library filename and a package name.
 *
 * @param lib_filename The filename of the library.
 * @param pkg_name The name of the package.
 * @return true If a match is likely.
 * @return false Otherwise.
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
 * @brief Extracts a CVSS score from a severity string.
 *
 * @param severity_str The severity string (e.g., "7.5" or
 * "CVSS:3.1/AV:N/AC:L/PR:N/UI:N/S:U/C:H/I:H/A:H").
 * @return double The extracted CVSS score.
 */
double extract_cvss_score(const std::string &severity_str) {
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
 * @brief Checks for updates of depdiscover itself.
 */
void check_for_updates() {
  std::cerr << "[Info] Checking for updates...\n";
  try {
    const std::string repo_url(rz::config::PROJECT_HOMEPAGE_URL);
    const std::string current_version(rz::config::VERSION);

    if (!repo_url.empty() && !current_version.empty()) {
      auto result = ghupdate::check_github_update(repo_url, current_version);
      if (result.hasUpdate) {
        std::println("\n[Update] A new version is available: {} (current: {})",
                     result.latestVersion, current_version);
        std::println("[Update] Download: {}\n", repo_url);
      } else {
        std::println("\n[Update] {} is up to date.\n",
                     rz::config::EXECUTABLE_NAME);
      }
    }
  } catch (const std::exception &ex) {
    std::cerr << "[Error] Update check failed: " << ex.what() << "\n";
  }
}

/**
 * @brief Prints the help message to the standard error stream.
 *
 * @param program_name The name of the executable.
 */
void print_help(const char *program_name) {
  std::cerr
      << "Usage: " << program_name << " [OPTIONS]\n\n"
      << "Options:\n"
      << "  -c, --compile-commands <PATH>  Input: compile_commands.json\n"
      << "  -l, --libs <PATH>              Input: libs.txt (CMake generated)\n"
      << "  -b, --binary <PATH>            Input: Binary (for ldd analysis)\n"
      << "  -v, --vcpkg <PATH>             Input: vcpkg.json\n"
      << "  -C, --conan <PATH>             Input: conanfile.txt\n"
      << "  -m, --cmake <PATH>             Input: CMakeLists.txt (to find "
         "FetchContent)\n"
      << "  -o, --output <PATH>            Output: JSON file (Default: "
         "depdiscover.json)\n"
      << "  -n, --name <NAME>              Sets the project name in the "
         "report\n"
      << "  -e, --ecosystem <NAME>         OSV Ecosystem for CVE checks "
         "(Default: Debian)\n"
      << "  -H, --html <PATH>              Output: Generate HTML report "
         "(Optional)\n"
      << "  -x, --cyclonedx <PATH>         Output: Generate CycloneDX 1.4 JSON "
         "(Optional)\n"
      << "  -f, --fail-on-cvss <SCORE>     Build Breaker: Exit 1 if "
         "CVSS-Score >= SCORE (e.g., 7.0)\n"
      << "  -s, --suppressions <PATH>      Input: Path to JSON file with "
         "suppressed CVEs (Optional)\n"
      << "  --check-version                Checks for updates of depdiscover\n"
      << "  --version                      Shows the current version\n"
      << "  -h, --help                     Shows this help message\n\n"
      << "Info:\n"
      << "  " << rz::config::PROG_LONGNAME << "\n"
      << "  Version: " << rz::config::VERSION << "\n"
      << "  Author:  " << rz::config::AUTHOR << "\n";
}

/**
 * @brief Main entry point of the application.
 *
 * @param argc Number of command-line arguments.
 * @param argv Array of command-line arguments.
 * @return int Exit code (0 for success, non-zero for failure).
 */
int main(int argc, char **argv) {
  // --- Initialize libcurl ---
  curl_global_init(CURL_GLOBAL_DEFAULT);

  bool only_version = false;
  bool only_check = false;

  std::string cc_path = "compile_commands.json";
  std::string libs_txt_path = "libs.txt";
  std::string binary_path = "";
  std::string vcpkg_path = "vcpkg.json";
  std::string conan_path = "conanfile.txt";
  std::string cmake_lists_path = "CMakeLists.txt";

  std::string output_path = "";
  std::string project_name = "Unknown Project";
  std::string ecosystem = "Debian";
  std::string html_path = "";
  std::string cyclonedx_path = "";

  std::string suppressions_path = "";
  std::map<std::string, std::string> suppressions;

  double fail_on_cvss = 11.0;

  // Argument Parsing
  for (int i = 1; i < argc; ++i) {
    std::string arg = argv[i];
    if (arg == "-h" || arg == "--help") {
      print_help(argv[0]);
      curl_global_cleanup();
      return 0;
    } else if (arg == "--version") {
      only_version = true;
    } else if (arg == "--check-version") {
      only_check = true;
    } else if (arg == "-c" || arg == "--compile-commands") {
      if (i + 1 < argc)
        cc_path = argv[++i];
      else {
        std::cerr << "Error: " << arg << " requires a path.\n";
        return 1;
      }
    } else if (arg == "-l" || arg == "--libs") {
      if (i + 1 < argc)
        libs_txt_path = argv[++i];
      else {
        std::cerr << "Error: " << arg << " requires a path.\n";
        return 1;
      }
    } else if (arg == "-b" || arg == "--binary") {
      if (i + 1 < argc)
        binary_path = argv[++i];
      else {
        std::cerr << "Error: " << arg << " requires a path.\n";
        return 1;
      }
    } else if (arg == "-v" || arg == "--vcpkg") {
      if (i + 1 < argc)
        vcpkg_path = argv[++i];
      else {
        std::cerr << "Error: " << arg << " requires a path.\n";
        return 1;
      }
    } else if (arg == "-C" || arg == "--conan") {
      if (i + 1 < argc)
        conan_path = argv[++i];
      else {
        std::cerr << "Error: " << arg << " requires a path.\n";
        return 1;
      }
    } else if (arg == "-m" || arg == "--cmake") {
      if (i + 1 < argc)
        cmake_lists_path = argv[++i];
      else {
        std::cerr << "Error: " << arg << " requires a path.\n";
        return 1;
      }
    } else if (arg == "-o" || arg == "--output") {
      if (i + 1 < argc)
        output_path = argv[++i];
      else {
        std::cerr << "Error: " << arg << " requires a path.\n";
        return 1;
      }
    } else if (arg == "-n" || arg == "--name") {
      if (i + 1 < argc)
        project_name = argv[++i];
      else {
        std::cerr << "Error: " << arg << " requires a path.\n";
        return 1;
      }
    } else if (arg == "-e" || arg == "--ecosystem") {
      if (i + 1 < argc)
        ecosystem = argv[++i];
      else {
        std::cerr << "Error: " << arg << " requires an ecosystem name.\n";
        return 1;
      }
    } else if (arg == "-H" || arg == "--html") {
      if (i + 1 < argc)
        html_path = argv[++i];
      else {
        std::cerr << "Error: " << arg << " requires a path.\n";
        return 1;
      }
    } else if (arg == "-x" || arg == "--cyclonedx") {
      if (i + 1 < argc)
        cyclonedx_path = argv[++i];
      else {
        std::cerr << "Error: " << arg << " requires a path.\n";
        return 1;
      }
    } else if (arg == "-f" || arg == "--fail-on-cvss") {
      if (i + 1 < argc) {
        try {
          fail_on_cvss = std::stod(argv[++i]);
        } catch (...) {
          std::cerr
              << "Error: --fail-on-cvss requires a valid number (e.g., 7.0).\n";
          return 1;
        }
      } else {
        std::cerr << "Error: " << arg << " requires a score.\n";
        return 1;
      }
    } else if (arg == "-s" || arg == "--suppressions") {
      if (i + 1 < argc)
        suppressions_path = argv[++i];
      else {
        std::cerr << "Error: " << arg << " requires a path.\n";
        return 1;
      }
    }
  }

  if (only_version) {
    std::println("{} version {}", rz::config::EXECUTABLE_NAME,
                 rz::config::VERSION);
    curl_global_cleanup();
    return 0;
  }

  if (only_check) {
    check_for_updates();
    curl_global_cleanup();
    return 0;
  }

  // --- Handle Defaults and Data Directory ---
  std::string date_prefix = get_current_date() + "_" + get_platform_name();
  bool use_data_dir = false;

  if (output_path.empty()) {
    output_path = "data/reports/" + date_prefix + "_depdiscover.json";
    use_data_dir = true;
  }
  if (html_path.empty()) {
    // If user wants HTML (by default or via flag? User says "all file-outputs")
    // I assume standard paths should be set for all possible outputs if not
    // specified.
    html_path = "data/reports/" + date_prefix + "_depdiscover.html";
    use_data_dir = true;
  }
  if (cyclonedx_path.empty()) {
    cyclonedx_path = "data/reports/" + date_prefix + "_CycloneDx.json";
    use_data_dir = true;
  }

  if (use_data_dir) {
    std::error_code ec;
    if (!fs::exists("data/reports", ec)) {
      fs::create_directories("data/reports", ec);
    }
  }

  try {
    // --- 0. Load Suppressions ---
    if (!suppressions_path.empty() && fs::exists(suppressions_path)) {
      std::cerr << "[Info] Loading suppressions from: " << suppressions_path
                << "\n";
      try {
        std::ifstream f(suppressions_path);
        json j;
        f >> j;
        for (auto &[key, value] : j.items()) {
          suppressions[key] = value.get<std::string>();
        }
      } catch (const std::exception &e) {
        std::cerr << "[Warning] Error reading suppressions file: " << e.what()
                  << "\n";
      }
    }

    // --- 1. Load Dependencies ---
    std::vector<Dependency> deps;

    if (fs::exists(vcpkg_path)) {
      std::cerr << "[Info] Loading Vcpkg manifest: " << vcpkg_path << "\n";
      auto v = parse_vcpkg_manifest(vcpkg_path);
      deps.insert(deps.end(), v.begin(), v.end());
    }
    if (fs::exists(conan_path)) {
      std::cerr << "[Info] Loading Conan file: " << conan_path << "\n";
      auto c = parse_conan_dependencies(conan_path);
      deps.insert(deps.end(), c.begin(), c.end());
    }
    if (fs::exists(libs_txt_path)) {
      std::cerr << "[Info] Loading CMake libs.txt: " << libs_txt_path << "\n";
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

    if (fs::exists(cmake_lists_path)) {
      std::cerr << "[Info] Checking for FetchContent in: " << cmake_lists_path
                << "\n";
      auto fetch_deps = parse_cmake_fetch_content(cmake_lists_path);
      for (const auto &info : fetch_deps) {
        bool found = false;
        for (auto &existing : deps) {
          // Case-insensitive match or contains match for FetchContent
          if (string_contains(existing.name, info.name) ||
              string_contains(info.name, existing.name)) {
            found = true;
            if (existing.version == "latest" || existing.version == "unknown") {
              existing.version = info.version;
              if (existing.source.empty() || existing.source == "manifest") {
                existing.source = "cmake_fetchcontent";
              }
            }
            break;
          }
        }
        if (!found) {
          Dependency d;
          d.name = info.name;
          d.version = info.version;
          d.type = "cmake_fetch";
          d.source = "cmake_fetchcontent";
          deps.push_back(d);
        }
      }
      // Additional Export as requested
      if (!fetch_deps.empty()) {
        std::string csv_path = "data/reports/" + date_prefix + "_gh-libs.csv";
        std::string json_path = "data/reports/" + date_prefix + "_gh-libs.json";

        // Ensure data directory exists (just in case it was only for this
        // output)
        std::error_code ec;
        if (!fs::exists("data/reports", ec))
          fs::create_directories("data/reports", ec);

        export_fetch_to_csv(fetch_deps, csv_path);
        export_fetch_to_json(fetch_deps, json_path);
        std::cerr << "[Success] Exported FetchContent to " << csv_path
                  << " and " << json_path << "\n";
      }
    }

    // --- 2. Scan Build Artifacts ---
    std::set<std::string> all_resolved_headers;
    std::set<std::string> all_elf_libs;

    if (fs::exists(cc_path)) {
      std::cerr << "[Info] Analyzing Compile Commands: " << cc_path << "\n";
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
                << " header files identified.\n";
    } else {
      std::cerr << "[Info] Skip Compile Commands analysis (file not found: "
                << cc_path << ")\n";
    }

    if (!binary_path.empty()) {
      std::cerr << "[Info] Scanning binary (ELF): " << binary_path << "\n";
      auto l = scan_elf_dependencies(binary_path);
      all_elf_libs.insert(l.begin(), l.end());
    }

    // --- 3. Mapping & Enrichment ---
    std::cerr << "[Info] Starting mapping & metadata enrichment...\n";

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

      dep.cves = query_cves(dep.name, clean_ver, ecosystem);

      // Apply suppressions
      if (!suppressions.empty()) {
        for (auto &cve : dep.cves) {
          if (suppressions.contains(cve.id)) {
            cve.suppressed = true;
            cve.suppression_reason = suppressions[cve.id];
          }
        }
      }
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

    // --- 5. Generate Output ---
    json root;

    json header;
    header["schema_version"] = SCHEMA_VERSION;
    header["scan_date"] = get_current_date();

    json tool;
    tool["name"] = rz::config::PROJECT_NAME;
    tool["version"] = rz::config::VERSION;
    tool["description"] = rz::config::PROG_LONGNAME;
    tool["homepage"] = rz::config::PROJECT_HOMEPAGE_URL;
    tool["author"] = rz::config::AUTHOR;
    header["tool"] = tool;

    json project;
    project["name"] = project_name;
    project["workspace_root"] = fs::current_path().string();
    header["project"] = project;

    root["header"] = header;
    root["dependencies"] = deps;

    // Save JSON
    std::ofstream out_file(output_path);
    if (!out_file) {
      std::cerr << "Error: Could not write output file: " << output_path
                << "\n";
      return 1;
    }
    out_file << root.dump(2);
    std::cerr << "[Success] SBOM report written to: " << output_path << "\n";

    // Generate HTML Report
    if (!html_path.empty()) {
      generate_html_report(root, html_path);
      std::cerr << "[Success] HTML report written to: " << html_path << "\n";
    }

    // Generate CycloneDX Report
    if (!cyclonedx_path.empty()) {
      generate_cyclonedx_report(root, cyclonedx_path);
      std::cerr << "[Success] CycloneDX SBOM written to: " << cyclonedx_path
                << "\n";
    }

    // --- 6. Check Build Breaker Logic ---
    if (fail_on_cvss <= 10.0) {
      bool critical_vuln_found = false;
      std::cerr << "\n[Audit] Checking for vulnerabilities with CVSS >= "
                << fail_on_cvss << " ...\n";

      for (const auto &dep : deps) {
        for (const auto &cve : dep.cves) {
          // Suppressed CVEs are ignored in build breaker
          if (!cve.suppressed && cve.id != "SAFE" && cve.id != "NOT-CHECKED" &&
              cve.id != "CHECK-ERROR") {
            double score = extract_cvss_score(cve.severity);
            if (score >= fail_on_cvss) {
              std::cerr << "  ❌ ERROR: " << dep.name << " v" << dep.version
                        << " has vulnerability " << cve.id << " (Score: ~"
                        << score << ")\n";
              critical_vuln_found = true;
            }
          }
        }
      }

      if (critical_vuln_found) {
        std::cerr << "\n[Audit] BUILD FAILED: Critical vulnerabilities found "
                     "exceeding threshold!\n";
        return 1; // Exit with error code
      } else {
        std::cerr << "[Audit] BUILD SUCCESS: No critical unresolved "
                     "vulnerabilities found above threshold.\n";
      }
    }

  } catch (const std::exception &ex) {
    std::cerr << "Error: " << ex.what() << "\n";
    curl_global_cleanup();
    return 1;
  }

  curl_global_cleanup();
  return 0;
}
