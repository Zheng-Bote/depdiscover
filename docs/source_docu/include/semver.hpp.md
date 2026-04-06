# semver.hpp

## File Header Information

| Field | Value |
| :--- | :--- |
| **SPDX Comment** | Semantic Versioning Utility |
| **SPDX Type** | SOURCE |
| **Contributor** | ZHENG Robert |
| **License ID** | MIT |
| **File** | `semver.hpp` |
| **Description** | Utilities for cleaning and parsing semantic version strings. |
| **Version** | 1.0.0 |
| **Date** | 2026-02-18 |
| **Author** | ZHENG Robert (robert@hase-zheng.net) |
| **Copyright** | Copyright (c) 2026 ZHENG Robert |
| **License** | MIT License |

<!-- START doctoc generated TOC please keep comment here to allow auto update -->
<!-- DON'T EDIT THIS SECTION, INSTEAD RE-RUN doctoc TO UPDATE -->
**Table of Contents**

- [API Documentation](#api-documentation)
  - [`inline std::string clean_version(const std::string &raw_version) `](#inline-stdstring-clean_versionconst-stdstring-raw_version-)

<!-- END doctoc generated TOC please keep comment here to allow auto update -->

## API Documentation

### `inline std::string clean_version(const std::string &raw_version) `

> Cleans a version string to extract "Major.Minor.Patch".

| Parameter | Description |
| --- | --- |
| `raw_version` | The raw version string. |

**Returns:** std::string The cleaned version string, or original if no pattern matched.

---

