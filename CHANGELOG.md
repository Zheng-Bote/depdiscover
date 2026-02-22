# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

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
