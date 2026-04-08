# types.hpp

## File Header Information

| Field | Value |
| :--- | :--- |
| **SPDX Comment** | Common Types and Structures |
| **SPDX Type** | SOURCE |
| **Contributor** | ZHENG Robert |
| **License ID** | MIT |
| **File** | `types.hpp` |
| **Description** | definitions of common data structures like Dependency and CVE. |
| **Version** | 1.1.0 |
| **Date** | 2026-02-22 |
| **Author** | ZHENG Robert (robert@hase-zheng.net) |
| **Copyright** | Copyright (c) 2026 ZHENG Robert |
| **License** | MIT License |

<!-- START doctoc generated TOC please keep comment here to allow auto update -->
<!-- DON'T EDIT THIS SECTION, INSTEAD RE-RUN doctoc TO UPDATE -->
**Table of Contents**

- [API Documentation](#api-documentation)
  - [`inline double extract_cvss_score(const std::string &severity_str) `](#inline-double-extract_cvss_scoreconst-stdstring-severity_str-)
  - [`struct CVE `](#struct-cve-)
  - [`inline void to_json(nlohmann::json &j, const CVE &c) `](#inline-void-to_jsonnlohmannjson-j-const-cve-c-)
  - [`struct Dependency `](#struct-dependency-)
  - [`inline void to_json(nlohmann::json &j, const Dependency &d) `](#inline-void-to_jsonnlohmannjson-j-const-dependency-d-)

<!-- END doctoc generated TOC please keep comment here to allow auto update -->

## API Documentation

### `inline double extract_cvss_score(const std::string &severity_str) `

> Extracts a CVSS score from a severity string.

| Parameter | Description |
| --- | --- |
| `severity_str` | The severity string (e.g., "7.5" or |

**Returns:** double The extracted CVSS score.

---

### `struct CVE `

> Represents a Common Vulnerability and Exposure (CVE).

---

### `inline void to_json(nlohmann::json &j, const CVE &c) `

> Serializes a CVE object to JSON.

| Parameter | Description |
| --- | --- |
| `j` | The JSON object to write to. |
| `c` | The CVE object. |

---

### `struct Dependency `

> Represents a software dependency.

---

### `inline void to_json(nlohmann::json &j, const Dependency &d) `

> Serializes a Dependency object to JSON.

| Parameter | Description |
| --- | --- |
| `j` | The JSON object to write to. |
| `d` | The Dependency object. |

---

