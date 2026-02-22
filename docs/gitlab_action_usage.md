<!-- START doctoc generated TOC please keep comment here to allow auto update -->
<!-- DON'T EDIT THIS SECTION, INSTEAD RE-RUN doctoc TO UPDATE -->
**Table of Contents**

- [Using depdiscover AppImage in GitLab Actions](#using-depdiscover-appimage-in-gitlab-actions)
  - [GitLab Integration Benefits](#gitlab-integration-benefits)
  - [Workflow Example](#workflow-example)
  - [Why Extract the AppImage?](#why-extract-the-appimage)
    - [Key Steps](#key-steps)

<!-- END doctoc generated TOC please keep comment here to allow auto update -->

# Using depdiscover AppImage in GitLab Actions

This guide explains how to integrate the **depdiscover** AppImage into your GitLab Actions CI/CD pipeline to automatically generate SBOM reports and scan for vulnerabilities.

## GitLab Integration Benefits

- **Native SBOM Support**: GitLab can parse CycloneDX reports directly, providing a built-in UI for dependency visualization.
- **Security Dashboard**: Critical vulnerabilities are automatically surfaced in the Security & Compliance section.
- **Artifact Management**: Easy storage and retrieval of build artifacts.

## Workflow Example

You can use the AppImage directly in your workflow by extracting it. This is useful in environments where FUSE is not available (which is common in many containerized CI environments).

```yaml
dependency_scan:
  stage: test
  image: ubuntu:latest
  before_script:
    - apt-get update && apt-get install -y curl build-essential cmake
  script:
    # Build Project
    - mkdir build && cd build
    - cmake .. -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
    - cmake --build .
    - cd ..
    # Download AppImage
    - curl -L -o depdiscover.AppImage https://github.com/Zheng-Bote/depdiscover/releases/latest/download/depdiscover-x86_64.AppImage
    - chmod +x depdiscover.AppImage
    # Run Scanner
    - ./depdiscover.AppImage --appimage-extract
    - export LD_LIBRARY_PATH=$PWD/squashfs-root/usr/lib:$LD_LIBRARY_PATH
    - >
      ./squashfs-root/AppRun 
      --name "GitLab_Project" 
      --compile-commands ./build/compile_commands.json 
      --fail-on-cvss 7.0
      --output sbom.json 
      --cyclonedx gl-sbom.cdx.json 
      --html report.html
  artifacts:
    when: always
    paths:
      - sbom.json
      - gl-sbom.cdx.json
      - report.html
    reports:
      cyclonedx: gl-sbom.cdx.json # GitLab liest CycloneDX nativ aus!
```

## Why Extract the AppImage?

GitLab Actions runners often lack the necessary FUSE setup to run AppImages directly (e.g., `./depdiscover.AppImage`). Using the `--appimage-extract` flag bypasses this requirement by unpacking the application into a `squashfs-root` directory.

### Key Steps

1. **Extraction**: `./depdiscover.AppImage --appimage-extract`
2. **Environment**: Updating `LD_LIBRARY_PATH` ensures that any bundled shared libraries within the AppImage are correctly linked at runtime.
3. **Execution**: Calling `./squashfs-root/AppRun` starts the application with the same environment it would have had inside the mounted AppImage.
