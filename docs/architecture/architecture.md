<!-- START doctoc generated TOC please keep comment here to allow auto update -->
<!-- DON'T EDIT THIS SECTION, INSTEAD RE-RUN doctoc TO UPDATE -->
**Table of Contents**

- [Architecture Documentation](#architecture-documentation)
  - [Overview](#overview)
  - [Component Diagram](#component-diagram)
  - [Sequence Diagram](#sequence-diagram)
  - [Key Parameters](#key-parameters)

<!-- END doctoc generated TOC please keep comment here to allow auto update -->

# Architecture Documentation

## Overview

**depdiscover** is designed as a modular dependency scanner. It follows a multi-stage pipeline:

1.  **Input Parsing**: Gathering data from various sources (manifests, build systems, binaries).
2.  **Physical Scanning**: Identifying actual files and symbols used in the build.
3.  **Metadata Enrichment**: Resolving licenses, querying vulnerabilities, and mapping headers.
4.  **Reporting**: Generating structured JSON and interactive HTML reports.

## Component Diagram

The following diagram illustrates the relationship between the different modules of the system.

```mermaid
graph TD
    subgraph Inputs
        CC[compile_commands.json]
        LIBS[libs.txt]
        MAN[vcpkg.json / conanfile]
        BIN[Binary ELF]
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
        JSON["depdiscover.json <br> (SBOM)"]
        HTML["report.html <br> (Interactive)"]
    end

    CC --> P_CC
    LIBS --> P_CMAKE
    MAN --> P_MAN
    BIN --> S_ELF

    P_CC --> S_INC
    S_INC --> R_HEAD

    P_MAN --> R_PKG
    P_CMAKE --> R_PKG

    R_HEAD --> R_LIC
    R_PKG --> R_CVE

    P_CC & P_CMAKE & P_MAN & S_ELF & R_HEAD & R_LIC & R_CVE --> JSON
    JSON --> HTML
```

## Sequence Diagram

The sequence of operations during a typical scan:

```mermaid
sequenceDiagram
    participant User
    participant Main
    participant Parsers as Manifest/Build Parsers
    participant Scanners as Binary/Include Scanners
    participant Resolvers as Resolvers (License/CVE/PkgConfig)
    participant Output as Output (JSON/HTML)

    User->>Main: Execute with options (-e, -H, etc.)
    Main->>Parsers: parse_vcpkg_manifest()
    Main->>Parsers: parse_conan_dependencies()
    Main->>Parsers: parse_cmake_libs()
    Main->>Scanners: load_compile_commands()
    Main->>Scanners: scan_includes()
    Main->>Scanners: scan_elf_dependencies()
    Main->>Resolvers: query(pkg_name) via PkgConfig
    Main->>Resolvers: resolve_licenses()
    Main->>Resolvers: query_cves(name, version, ecosystem)
    Main->>Output: JSON report
    Main->>Output: generate_html_report()
    Output-->>User: Report files
```

## Key Parameters

- **Ecosystem (`-e`)**: Allows specifying the OSV ecosystem (e.g., "Debian", "Alpine", "Ubuntu") to improve CVE matching accuracy.
- **HTML Report (`-H`)**: Generates an interactive HTML summary for easier human inspection.
