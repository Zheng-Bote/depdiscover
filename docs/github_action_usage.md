<!-- START doctoc generated TOC please keep comment here to allow auto update -->
<!-- DON'T EDIT THIS SECTION, INSTEAD RE-RUN doctoc TO UPDATE -->

**Table of Contents**

- [Using depdiscover GitHub Action](#using-depdiscover-github-action)
  - [Workflow Example](#workflow-example)
    - [Standard Integration](#standard-integration)
  - [Inputs](#inputs)
  - [Advanced: Manual AppImage Usage](#advanced-manual-appimage-usage)

<!-- END doctoc generated TOC please keep comment here to allow auto update -->

# Using depdiscover GitHub Action

This guide explains how to integrate the **depdiscover** GitHub Action into your CI/CD pipeline to automatically generate SBOM reports, scan for vulnerabilities, and track GitHub-managed dependencies.

## Workflow Example

The easiest way to use **depdiscover** in GitHub Actions is by using the provided action in this repository.

### Standard Integration

Add a file like `.github/workflows/depdiscover.yml` to your repository:

```yaml
name: Dependency Scan (depdiscover)

on:
  push:
    branches: [main, master]
  pull_request:
    branches: [main, master]

permissions:
  contents: write

jobs:
  scan:
    runs-on: ubuntu-latest
    steps:
      - name: Checkout code
        uses: actions/checkout@v4

      - name: Install build dependencies (Conan)
        run: |
          sudo apt-get update
          sudo apt-get install -y wget pipx cmake ninja-build build-essential
          pipx install conan
          conan profile detect --force

      - name: Build Project
        # Ensure compile_commands.json and libs.txt are generated using Conan
        run: |
          conan install . --output-folder=build --build=missing
          cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=build/conan_toolchain.cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
          cmake --build build

      - name: Run depdiscover Action
        uses: Zheng-Bote/depdiscover@v1.5.0
        with:
          project-name: "MyProject"
          compile-commands: "build/compile_commands.json"
          libs: "build/libs.txt"
          cmake: "CMakeLists.txt"
          fail-on-cvss: 7.0
          html: "data/reports/report.html"
          cyclonedx: "data/reports/bom.cdx.json"
          output: "data/reports/report.json"

      - name: Upload SBOM and Security Reports
        if: always()
        uses: actions/upload-artifact@v4
        with:
          name: depdiscover-reports
          path: data/reports/

      - name: Commit depdiscover reports
        if: always()
        run: |
          git config user.name "github-actions[bot]"
          git config user.email "github-actions[bot]@users.noreply.github.com"

          # Reports hinzufügen
          git add data/reports || true

          # Nur committen, wenn es Änderungen gibt
          if ! git diff --cached --quiet; then
            git commit -m "Add depdiscover reports [skip ci]"
            git push
          else
            echo "No changes to commit."
          fi
```

## Inputs

The following inputs are available for the action:

| Input              | Description                                    | Default                               |
| ------------------ | ---------------------------------------------- | ------------------------------------- |
| `compile-commands` | Path to `compile_commands.json`                | `compile_commands.json`               |
| `libs`             | Path to CMake generated `libs.txt`             | `libs.txt`                            |
| `binary`           | Path to the executable binary for ELF analysis | -                                     |
| `vcpkg`            | Path to `vcpkg.json` manifest                  | `vcpkg.json`                          |
| `conan`            | Path to `conanfile.txt`                        | `conanfile.txt`                       |
| `cmake`            | Path to `CMakeLists.txt` (for FetchContent)    | `CMakeLists.txt`                      |
| `output`           | Path for the generated JSON file               | (auto-generated in `./data/reports/`) |
| `html`             | Path for the generated HTML report             | (auto-generated in `./data/reports/`) |
| `cyclonedx`        | Path for the generated CycloneDX 1.4 SBOM      | (auto-generated in `./data/reports/`) |
| `ecosystem`        | OSV Ecosystem for CVE checks                   | `Debian`                              |
| `fail-on-cvss`     | Build Breaker: Exit 1 if CVSS-Score >= SCORE   | -                                     |
| `suppressions`     | Path to JSON file with suppressed CVEs         | -                                     |
| `project-name`     | Project name in the report header              | `GitHub Action Project`               |

## Advanced: Manual AppImage Usage

If you prefer not to use the composite action, you can run the AppImage manually. This is useful if you need custom FUSE settings or specific environment variables.

```yaml
- name: Download and Run AppImage
  run: |
    wget -q https://github.com/Zheng-Bote/depdiscover/releases/download/v1.5.0/depdiscover-x86_64.AppImage
    chmod +x depdiscover-x86_64.AppImage
    ./depdiscover-x86_64.AppImage --appimage-extract-and-run -c build/compile_commands.json -f 7.0
```
