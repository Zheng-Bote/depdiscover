# depdiscover

**Native C++ Dependency Scanner & SBOM Generator**

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![C++23](https://img.shields.io/badge/C%2B%2B-23-blue.svg)]()
[![CMake](https://img.shields.io/badge/CMake-3.23+-blue.svg)]()

[![GitHub release (latest by date)](https://img.shields.io/github/v/release/Zheng-Bote/depdiscover?logo=GitHub)](https://github.com/Zheng-Bote/depdiscover/releases)

[Report Issue](https://github.com/Zheng-Bote/depdiscover/issues) · [Request Feature](https://github.com/Zheng-Bote/depdiscover/pulls)

---

<!-- START doctoc generated TOC please keep comment here to allow auto update -->
<!-- DON'T EDIT THIS SECTION, INSTEAD RE-RUN doctoc TO UPDATE -->
**Table of Contents**

- [Description](#description)
- [🚀 Key Features](#-key-features)
- [Screenshots](#screenshots)
- [🏗 Architecture](#-architecture)
- [🛠 Prerequisites](#-prerequisites)
  - [Runtime Requirements](#runtime-requirements)
- [📦 Build Instructions](#-build-instructions)
- [💻 Usage](#-usage)
  - [Basic Command](#basic-command)
  - [Full Example](#full-example)
  - [Options](#options)
  - [💡 Generating libs.txt (CMake Integration)](#-generating-libstxt-cmake-integration)
  - [📄 Output Example](#-output-example)
- [🛡️ CI/CD & Build Breaker](#-cicd--build-breaker)
- [🤫 Suppressions (Ignore Vulnerabilities)](#-suppressions-ignore-vulnerabilities)
- [📜 License](#-license)
- [📄 Changelog](#-changelog)
- [Author](#author)
- [Code Contributors](#code-contributors)

<!-- END doctoc generated TOC please keep comment here to allow auto update -->

---

## Description

**depdiscover** is a lightweight, high-performance tool designed to bridge the gap between declared dependencies (manifests) and physical build artifacts. It scans your C++ project's build environment, analyzes binaries, checks for licenses, and queries vulnerabilities (CVEs) to generate a comprehensive Software Bill of Materials (SBOM) in JSON format.

## 🚀 Key Features

- **Hybrid Analysis**: Combines data from package manifests with actual build artifacts.
- **Multi-Source Parsing**:
  - Manifests: `vcpkg.json`, `conanfile.txt`
  - Build Data: `compile_commands.json` (Clang/CMake), `libs.txt` (CMake Targets)
  - Binaries: Native ELF scanner (analyzes `DT_NEEDED` / `ldd` equivalent)
- **Deep Inspection**:
  - Header Resolution: Maps logical includes to physical files on disk.
  - License Scanning: Detects licenses via static DB and file system heuristics.
  - Security (CVE): Live vulnerability check via OSV.dev API (using `curl`).
- **CI/CD Ready**: Configurable build breaker (`--fail-on-cvss`) to automatically fail pipelines on critical vulnerabilities.
- **Auditing**: Suppress false positives or accepted risks using a `.suppressions.json` file.
- **Structured Output**:
  - Generates a detailed custom JSON report including metadata, file paths, licenses, and security status.
  - Generates an industry-standard **CycloneDX 1.4 SBOM** in JSON format.
  - Generates an interactive **HTML Dashboard** featuring CVSS color-coding and detailed vulnerability links.

## Screenshots

_example_ HTML Report (see also `docs/example_report.html`):

![html_report](docs/img/overview_html-report.png)

_example_ CycloneDX SBOM: see `docs/example_sbom-cyclonedx.json`

> [!TIP]
> see also the Desktop App [qt_depdiscover_ui](https://github.com/Zheng-Bote/qt_depdiscover_ui)

## 🏗 Architecture

The tool operates in three stages: **Input Parsing**, **Physical Scanning**, and **Metadata Enrichment**.

For a detailed look at the system design, please refer to the [Full Documentation](docs/index.md) and the [Architecture Overview](docs/architecture/architecture.md).

```mermaid
graph TD
    subgraph External
        GH[GitHub API]
        OSV[OSV.dev API]
    end

    subgraph Inputs
        CC[compile_commands.json]
        LIBS[libs.txt]
        MAN[vcpkg.json / conanfile]
        BIN[Binary ELF]
        SUPP[suppressions.json]
    end

    subgraph Core
        UC[GitHub Update Checker]
        P_CC[Compile DB Parser]
        P_CMAKE[CMake Libs Parser]
        P_MAN[Manifest Parser]
        S_ELF[ELF Scanner]
        S_INC[Include Scanner]
        BB[Build Breaker / Audit]
    end

    subgraph Resolvers
        R_HEAD[Header Resolver]
        R_PKG[Pkg-Config]
        R_LIC[License Resolver]
        R_CVE["CVE Resolver<br>(OSV.dev via curl)"]
    end

    subgraph Output
        JSON["depdiscover.json <br> (Detailed SBOM)"]
        HTML["report.html <br> (Interactive HTML)"]
        CDX["bom.cdx.json <br> (CycloneDX 1.4)"]
    end

    GH <--> UC
    CC --> P_CC
    LIBS --> P_CMAKE
    MAN --> P_MAN
    BIN --> S_ELF
    SUPP --> BB

    P_CC --> S_INC
    S_INC --> R_HEAD

    P_MAN --> R_PKG
    P_CMAKE --> R_PKG

    R_HEAD --> R_LIC
    R_PKG --> R_CVE
    OSV <--> R_CVE

    P_CC & P_CMAKE & P_MAN & S_ELF & R_HEAD & R_LIC & R_CVE --> JSON
    JSON --> BB
    JSON --> HTML
    JSON --> CDX
```

## 🛠 Prerequisites

To build and run depdiscover, you need:

- **C++ Compiler**: Supporting C++23 (e.g., GCC 13+, Clang 16+, MSVC VS2022 17.6+).
- **CMake**: Version 3.23 or higher.
- **Dependencies**:
- nlohmann_json, gh_update_checker (usually handled via CMake/Vcpkg).

### Runtime Requirements

- curl (must be installed in the system PATH for CVE queries).
- pkg-config (optional, for better system library resolution).

## 📦 Build Instructions

```bash
# 1. Clone the repository
git clone https://github.com/your-org/depdiscover.git
cd depdiscover

# 2. Create build directory
mkdir build && cd build

# 3. Configure (ensure nlohmann_json is found)
cmake ..

# 4. Build
cmake --build .
```

## 💻 Usage

depdiscover is a command-line tool. You can provide various input files to increase the accuracy of the result.

### Basic Command

```bash
./depdiscover -c ./compile_commands.json -o sbom.json
```

### Full Example

To get the most comprehensive report, provide as many inputs as possible:

```bash
./depdiscover \
 --name "MyApplication" \
 --compile-commands ../build/compile_commands.json \
 --libs ../build/libs.txt \
 --binary ../build/bin/myapp \
 --vcpkg ../vcpkg.json \
 --ecosystem "Debian" \
 --suppressions suppressions.json \
 --fail-on-cvss 7.0 \
 --cyclonedx docs/bom.cdx.json \
 --html docs/report.html \
 --output docs/report.json
```

### Options

| Flag | Long               | Description                                                              |
| ---- | ------------------ | ------------------------------------------------------------------------ |
| -c   | --compile-commands | Path to compile_commands.json (Input).                                   |
| -l   | --libs             | Path to CMake generated libs.txt (Input).                                |
| -b   | --binary           | Path to the executable binary for ELF analysis (Input).                  |
| -v   | --vcpkg            | Path to vcpkg.json manifest (Input).                                     |
| -C   | --conan            | Path to conanfile.txt (Input).                                           |
| -o   | --output           | Path for the generated JSON file (Output).                               |
| -n   | --name             | Project name to appear in the report header.                             |
| -e   | --ecosystem        | OSV Ecosystem for CVE checks (Default: Debian).                          |
| -f   | --fail-on-cvss     | Build Breaker: Exit 1 if CVSS-Score greater or equal to SCORE (e.g. 7.0) |
| -H   | --html             | Path for the generated HTML report (Optional).                           |
| -x   | --cyclonedx        | Path for the generated CycloneDX 1.4 SBOM (Optional).                    |
| -s   | --suppressions     | Path to JSON file with suppressed CVEs (Optional).                       |
| -h   | --help             | Show help message.                                                       |

### 💡 Generating libs.txt (CMake Integration)

To enable the **--libs** parser, add the following to your project's CMakeLists.txt. This allows depdiscover to see CMake targets and header-only libraries that might not appear in the binary.

```cmake
# Add this at the end of your CMakeLists.txt
file(GENERATE OUTPUT "${CMAKE_BINARY_DIR}/libs.txt"
     CONTENT "$<TARGET_PROPERTY:YourTargetName,LINK_LIBRARIES>")
```

### 📄 Output Example

The generated JSON contains a metadata header and a list of dependencies including licenses and CVEs and paths.

```json
{
  "header": {
    "schema_version": "1.2",
    "scan_date": "2026-02-22",
    "tool": {
      "name": "depdiscover",
      "version": "1.2.0"
    },
    "project": {
      "name": "MyApplication",
      "workspace_root": "/home/user/projects/myapp"
    }
  },
  "dependencies": [
    {
      "name": "fmt",
      "version": "10.2.1",
      "type": "vcpkg",
      "source": "pkgconfig",
      "headers": ["/usr/include/fmt/core.h", "/usr/include/fmt/format.h"],
      "libraries": ["/usr/lib/x86_64-linux-gnu/libfmt.so.10"],
      "licenses": ["MIT"],
      "cves": [
        {
          "id": "SAFE",
          "severity": "NONE",
          "summary": "No vulnerabilities found in ecosystem 'Debian'. Checked on 2026-02-22",
          "fixed_version": "",
          "suppressed": false,
          "suppression_reason": ""
        }
      ]
    }
  ]
}
```

## 🛡️ CI/CD & Build Breaker

You can use depdiscover as a security gate in your CI/CD pipelines. By passing the --fail-on-cvss flag, the tool will exit with code 1 if it detects any unsuppressed vulnerability matching or exceeding the given score.

```bash
# Fail the pipeline if any "High" or "Critical" vulnerabilities are found
./depdiscover -c compile_commands.json --fail-on-cvss 7.0
```

## 🤫 Suppressions (Ignore Vulnerabilities)

In real-world projects, some CVEs might be false positives or affect components of a library you are not using. You can suppress these using a .json file passed via -s or --suppressions.

Create a file (e.g., suppressions.json) mapping the exact CVE ID to your justification:

```json
{
  "CVE-2023-12345": "Ignored because the vulnerable networking module is disabled during our build.",
  "GHSA-abcd-1234-wxyz": "False Positive: Our custom wrapper sanitizes the input before it reaches the library."
}
```

> [!NOTE]
> Suppressed vulnerabilities will not trigger the Build Breaker\
> and will be neatly grayed out and marked as "Suppressed" with your provided reason in the HTML report.

---

## 📜 License

This project is licensed under the MIT License - see the LICENSE file for details.

Copyright (c) 2026 ZHENG Robert

## 📄 Changelog

For a detailed history of changes, see the [CHANGELOG.md](CHANGELOG.md).

## Author

[![Zheng Robert - Core Development](https://img.shields.io/badge/Github-Zheng_Robert-black?logo=github)](https://www.github.com/Zheng-Bote)

## Code Contributors

![Contributors](https://img.shields.io/github/contributors/Zheng-Bote/depdiscover?color=dark-green)

---

**Happy checking! 🚀** :vulcan_salute:
