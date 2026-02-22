<!-- START doctoc generated TOC please keep comment here to allow auto update -->
<!-- DON'T EDIT THIS SECTION, INSTEAD RE-RUN doctoc TO UPDATE -->
**Table of Contents**

- [Using depdiscover AppImage in GitHub Actions](#using-depdiscover-appimage-in-github-actions)
  - [Workflow Example](#workflow-example)
  - [Why Extract the AppImage?](#why-extract-the-appimage)
    - [Key Steps:](#key-steps)

<!-- END doctoc generated TOC please keep comment here to allow auto update -->

# Using depdiscover AppImage in GitHub Actions

This guide explains how to integrate the **depdiscover** AppImage into your GitHub Actions CI/CD pipeline to automatically generate SBOM reports and scan for vulnerabilities.

## Workflow Example

You can use the AppImage directly in your workflow by extracting it. This is useful in environments where FUSE is not available (which is common in many containerized CI environments).

```yaml
name: Dependency Scan (depdiscover)

on:
  push:
    branches: [ main ]
  pull_request:
    branches: [ main ]

jobs:
  scan:
    runs-on: ubuntu-latest
    steps:
      - name: Checkout code
        uses: actions/checkout@v4

      - name: Build Project (Optional)
        run: |
          mkdir build && cd build
          cmake .. -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
          cmake --build .
          # Generate libs.txt if needed (see README.md)

      - name: Download depdiscover AppImage
        run: |
          # Replace with the actual URL of your release
          curl -L -o depdiscover.AppImage https://github.com/Zheng-Bote/depdiscover/releases/latest/download/depdiscover-x86_64.AppImage
          chmod +x depdiscover.AppImage

      - name: Extract and Run depdiscover
        env:
          PROJECT_NAME: "MyAwesomeProject"
          ECOSYSTEM: "Debian" # Default, or change to Ubuntu, etc.
        run: |
          # 1. Extract the AppImage
          ./depdiscover.AppImage --appimage-extract
          
          # 2. Set Library Path (ensure internal libs are found)
          export LD_LIBRARY_PATH=$PWD/squashfs-root/usr/lib:$LD_LIBRARY_PATH
          
          # 3. Run the tool via AppRun with desired parameters
          ./squashfs-root/AppRun 
            --name "$PROJECT_NAME" 
            --compile-commands ./build/compile_commands.json 
            --libs ./build/libs.txt 
            --ecosystem "$ECOSYSTEM" 
            --output sbom.json 
            --html report.html

      - name: Upload SBOM Report
        uses: actions/upload-artifact@v4
        with:
          name: depdiscover-reports
          path: |
            sbom.json
            report.html
```

## Why Extract the AppImage?

GitHub Actions runners often lack the necessary FUSE setup to run AppImages directly (e.g., `./depdiscover.AppImage`). Using the `--appimage-extract` flag bypasses this requirement by unpacking the application into a `squashfs-root` directory.

### Key Steps:
1.  **Extraction**: `./depdiscover.AppImage --appimage-extract`
2.  **Environment**: Updating `LD_LIBRARY_PATH` ensures that any bundled shared libraries within the AppImage are correctly linked at runtime.
3.  **Execution**: Calling `./squashfs-root/AppRun` starts the application with the same environment it would have had inside the mounted AppImage.
