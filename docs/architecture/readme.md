# Architecture Documentation

<!-- START doctoc generated TOC please keep comment here to allow auto update -->
<!-- DON'T EDIT THIS SECTION, INSTEAD RE-RUN doctoc TO UPDATE -->
**Table of Contents**

- [Overview](#overview)
- [Component Diagram](#component-diagram)
- [Sequence Diagram](#sequence-diagram)
- [Class Diagram](#class-diagram)
- [Key Features](#key-features)
- [Key Parameters](#key-parameters)

<!-- END doctoc generated TOC please keep comment here to allow auto update -->

## Overview

**depdiscover** is a modular dependency scanner and SBOM (Software Bill of Materials) generator for C++ projects. It identifies used libraries and headers by combining manifest parsing, build system analysis, and binary scanning.

The application follows a structured pipeline:

1. **Self-Check**: Verifies if a newer version of the tool is available on GitHub.
2. **Input Parsing**: Gathering data from various sources (vcpkg, Conan, CMake, Compile Commands).
3. **Physical Scanning**: Identifying actual files (headers) and symbols (binaries) used in the project.
4. **Metadata Enrichment**: Resolving licenses via heuristics and querying OSV.dev for vulnerabilities (CVEs).
5. **Audit & Compliance**: Applying vulnerability suppressions and enforcing CVSS threshold policies (Build Breaker).
6. **Reporting**: Generating structured JSON, interactive HTML, and industry-standard CycloneDX reports.

```mermaid
graph TD
    subgraph Inputs
        CC[compile_commands.json]
        LIBS[libs.txt]
        MAN[vcpkg.json / conanfile]
        BIN[Binary ELF]
        SUPP[suppressions.json]
    end

    subgraph Core
        P_CC[Compile DB Parser]
        P_CMAKE[CMake Libs Parser]
        P_MAN[Manifest Parser]
        S_ELF[ELF Scanner]
        S_INC[Include Scanner]
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
        MD["report.md <br> (Markdown Table)"]
        CDX["bom.cdx.json <br> (CycloneDX 1.4)"]
    end

    CC --> P_CC
    LIBS --> P_CMAKE
    MAN --> P_MAN
    BIN --> S_ELF
    SUPP -.-> R_CVE

    P_CC --> S_INC
    S_INC --> R_HEAD

    P_MAN --> R_PKG
    P_CMAKE --> R_PKG

    R_HEAD --> R_LIC
    R_PKG --> R_CVE

    P_CC & P_CMAKE & P_MAN & S_ELF & R_HEAD & R_LIC & R_CVE --> JSON
    JSON --> HTML
    JSON --> MD
    JSON --> CDX
```

## Component Diagram

The following diagram illustrates the relationship between the different modules and external services.

```mermaid
graph TD
    subgraph External
        GH[GitHub API]
        OSV[OSV.dev API]
        CN[Conan Center]
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
        JSON["depdiscover.json <br> (SBOM)"]
        HTML["report.html <br> (Interactive)"]
        MD["report.md <br> (Markdown)"]
        CDX["sbom-cyclonedx.json <br> (CycloneDX 1.4)"]
    end

    GH <--> UC
    CN <--> P_MAN
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
    JSON --> MD
    JSON --> CDX
```

## Sequence Diagram

The sequence of operations during a typical scan:

```mermaid
sequenceDiagram
    participant User
    participant Main
    participant GH as GitHub API
    participant Parsers as Manifest/Build Parsers
    participant Scanners as Binary/Include Scanners
    participant Resolvers as Resolvers (License/CVE/PkgConfig)
    participant OSV as OSV.dev API
    participant Output as Output (JSON/HTML/MD/CDX)

    User->>Main: Execute with options
    Main->>GH: check_github_update()
    GH-->>Main: Update info
    Main->>Main: Load Suppressions (-s)
    Main->>Parsers: parse_vcpkg_manifest()
    Main->>Parsers: parse_conan_dependencies()
    Main->>Parsers: parse_cmake_libs()
    Main->>Scanners: load_compile_commands()
    Main->>Scanners: scan_includes()
    Main->>Scanners: scan_elf_dependencies()
    Main->>Resolvers: query(pkg_name) via PkgConfig
    Main->>Resolvers: resolve_licenses()
    Main->>Resolvers: query_cves(name, version, ecosystem)
    Resolvers->>OSV: POST /v1/query
    OSV-->>Resolvers: Vulnerabilities
    Main->>Output: JSON report
    Main->>Output: generate_html_report()
    Main->>Output: generate_markdown_report()
    Main->>Output: generate_cyclonedx_report()
    Main->>Main: Build Breaker Check (-f)
    Note over Main: Exit 1 if critical<br>vulnerabilities found
    Output-->>User: Report files
```

## Class Diagram

The following diagram shows the key data structures and their relationships.

```mermaid
classDiagram
    class Dependency {
        +string name
        +string version
        +string type
        +string source
        +vector~string~ headers
        +vector~string~ libraries
        +vector~string~ licenses
        +vector~CVE~ cves
    }

    class CVE {
        +string id
        +string summary
        +string severity
        +double score
        +string fixed_version
        +bool suppressed
        +string suppression_reason
    }

    class PkgInfo {
        +bool found
        +string version
        +vector~string~ include_paths
        +vector~string~ lib_names
    }

    class PkgConfig {
        <<static>>
        +query(string package_name) PkgInfo
        -exec(string cmd) string
        -parse_flags(string input, string prefix) vector~string~
    }

    class SemVer {
        +int major
        +int minor
        +int patch
        +parse(string_view v) SemVer
    }

    class UpdateInfo {
        +bool hasUpdate
        +string latestVersion
    }

    Dependency "1" *-- "many" CVE
    PkgConfig ..> PkgInfo : creates
    Main ..> Dependency : manages
    Main ..> UpdateInfo : checks
```

## Key Features

- **Multi-Source Analysis**: Combines high-level manifests with low-level binary analysis for high accuracy.
- **Security Audit**: Automated vulnerability checking via OSV.dev.
- **Build Breaker**: Integration into CI/CD pipelines to block builds with critical vulnerabilities.
- **Compliance**: Identification of licenses and generation of CycloneDX 1.4 compliant SBOMs.

## Key Parameters

- **Ecosystem (`-e`)**: Allows specifying the OSV ecosystem (e.g., "Debian", "Alpine", "Ubuntu") to improve CVE matching accuracy.
- **Fail on CVSS (`-f`)**: Sets the threshold for the Build Breaker (e.g., 7.0 for High).
- **Suppressions (`-s`)**: Path to a JSON file to ignore specific CVEs (with reason).
- **HTML Report (`-H`)**: Generates an interactive HTML summary for human inspection.
- **CycloneDX Output (`-x`)**: Generates industry-standard SBOM for compliance workflows.
