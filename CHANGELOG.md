# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

---

<!-- START doctoc generated TOC please keep comment here to allow auto update -->
<!-- DON'T EDIT THIS SECTION, INSTEAD RE-RUN doctoc TO UPDATE -->
**Table of Contents**

- [[1.3.0] - 2026-04-06](#130---2026-04-06)
  - [Added](#added)
  - [Changed](#changed)
  - [Fixed](#fixed)
- [[1.2.1] - 2026-03-02](#121---2026-03-02)
...
<!-- END doctoc generated TOC please keep comment here to allow auto update -->

---

## [1.3.0] - 2026-04-06

### Added
- **CMake FetchContent Parser**: New `cmake_fetch_parser.hpp` to extract `FetchContent_Declare` dependencies (name, URL, version) directly from `CMakeLists.txt`.
- **Automatic GitHub Libs Export**: Automatically generates `gh-libs.csv` and `gh-libs.json` containing name, URL, and version of FetchContent-managed libraries.
- **Smart Default Output Paths**: If no output paths are specified, the tool now automatically saves all reports (`.json`, `.html`, `CycloneDx.json`, `gh-libs`) into a `./data/` directory (creating it if necessary).
- **Platform-Aware Filenames**: All default output files now feature a `<YYYY-MM-DD>_<Platform>_` prefix (e.g., `2026-04-06_Linux_depdiscover.json`).
- **New CLI Option**: Added `-m` / `--cmake <PATH>` to specify the location of the main `CMakeLists.txt` file.

### Changed
- **Network Stack**: Switched from `popen("curl")` shell calls to native `libcurl` C-API for improved performance, consistency, and thread-safety.
- **Initialization**: Added `curl_global_init`/`cleanup` for robust multi-threaded support.
- **Argument Parsing**: Improved CLI robustness with explicit parameter validation for all flags.
- **Header Files**: Moved FetchContent logic to a dedicated `cmake_fetch_parser.hpp` for better modularity.

### Fixed
- **Robustness**: Fixed a crash that occurred when the `compile_commands.json` file was missing; the tool now continues with available data.
- **Compiler Warnings**: Resolved warnings regarding ignored attributes in `unique_ptr` when using `pclose` by implementing a custom deleter.
- **ELF Scanning**: Improved safety in `elf_scanner.hpp` when reading the string table to prevent out-of-bounds access or issues with non-null-terminated strings.
- **Race Conditions**: Eliminated potential race conditions by removing the use of fixed temporary files for OSV queries.

## [1.2.1] - 2026-03-02

### Fixed
- **MacOS RPATH**: Corrected Mach-O headers and RPATH settings to ensure libraries are found correctly on macOS.
- **Linker Configuration**: Updated `libcurl` linking to use `CURL::libcurl` for better compatibility.
- **MacOS Dependency Scanning**: Fixed an issue where `LC_Path` was not found during ELF/Mach-O scanning on macOS.

### Changed
- Refined internal configuration and documentation for better clarity and consistency.

## [1.2.0] - 2026-02-22

### Added
- **CVE Suppression**: Added support for `.suppressions.json` to ignore or justify specific vulnerabilities.
- **Build Breaker**: Introduced `--fail-on-cvss` flag to fail CI/CD pipelines based on a CVSS score threshold (supports floating-point values).
- **CycloneDX Support**: Added generation of industry-standard CycloneDX 1.4 SBOMs in JSON format.
- **Update Checker**: Integrated automated checks for new versions of `depdiscover` via GitHub API.
- **AppImage Support**: Added scripts and configuration for creating modern AppImage packages.
- **GitHub Actions**: Provided ready-to-use workflows and documentation for CI/CD integration.
- **Interactive HTML Report**: Enhanced the HTML dashboard with CVSS color-coding and detailed vulnerability information.

### Changed
- Improved ELF scanner to more accurately resolve `DT_NEEDED` dependencies.
- Updated OSV.dev API integration for faster vulnerability lookups.
- Refactored internal architecture for better modularity (Resolvers vs. Parsers).

## [1.1.0] - 2026-01-15

### Added
- **Native ELF Scanner**: Direct analysis of binary executables to find linked libraries.
- **Multi-Source Parsing**: Support for `vcpkg.json`, `conanfile.txt`, `compile_commands.json`, and `libs.txt`.
- **License Detection**: Automated scanning for software licenses.
- **CVE Resolution**: Initial integration with OSV.dev for security vulnerability analysis.
- **HTML Generator**: Basic interactive HTML report for visualizing dependencies.

### Changed
- Standardized project structure (include/src/docs).
- Migrated build system to modern CMake with C++23 requirements.

## [1.0.0] - 2026-01-01

### Added
- Initial release of `depdiscover`.
- Core dependency tracking logic.
- Basic JSON output format.
- Support for `compile_commands.json` parsing.
