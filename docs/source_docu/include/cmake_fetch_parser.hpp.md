# cmake_fetch_parser.hpp

## File Header Information

| Field | Value |
| :--- | :--- |
| **SPDX Comment** | CMake FetchContent Parser |
| **SPDX Type** | SOURCE |
| **Contributor** | ZHENG Robert |
| **License ID** | MIT |
| **File** | `cmake_fetch_parser.hpp` |
| **Description** | Parses CMakeLists.txt to extract FetchContent_Declare information and export to CSV/JSON. |
| **Version** | 1.1.0 |
| **Date** | 2026-04-06 |
| **Author** | ZHENG Robert (robert@hase-zheng.net) |
| **Copyright** | Copyright (c) 2026 ZHENG Robert |
| **License** | MIT License |

<!-- START doctoc generated TOC please keep comment here to allow auto update -->
<!-- DON'T EDIT THIS SECTION, INSTEAD RE-RUN doctoc TO UPDATE -->
**Table of Contents**

- [API Documentation](#api-documentation)
  - [`struct FetchContentInfo `](#struct-fetchcontentinfo-)
  - [`inline std::vector<FetchContentInfo> parse_cmake_fetch_content(const std::string &cmake_path) `](#inline-stdvectorfetchcontentinfo-parse_cmake_fetch_contentconst-stdstring-cmake_path-)
  - [`inline void export_fetch_to_csv(const std::vector<FetchContentInfo>& deps, const std::string& filename) `](#inline-void-export_fetch_to_csvconst-stdvectorfetchcontentinfo-deps-const-stdstring-filename-)
  - [`inline void export_fetch_to_json(const std::vector<FetchContentInfo>& deps, const std::string& filename) `](#inline-void-export_fetch_to_jsonconst-stdvectorfetchcontentinfo-deps-const-stdstring-filename-)

<!-- END doctoc generated TOC please keep comment here to allow auto update -->

## API Documentation

### `struct FetchContentInfo `

> Structure to hold FetchContent dependency information.

---

### `inline std::vector<FetchContentInfo> parse_cmake_fetch_content(const std::string &cmake_path) `

> Extracts FetchContent_Declare blocks from a CMake file.

| Parameter | Description |
| --- | --- |
| `cmake_path` | Path to the CMakeLists.txt file. |

**Returns:** std::vector<FetchContentInfo> List of extracted dependency info.

---

### `inline void export_fetch_to_csv(const std::vector<FetchContentInfo>& deps, const std::string& filename) `

> Exports FetchContent info to a CSV file.

| Parameter | Description |
| --- | --- |
| `deps` | List of FetchContent dependencies. |
| `filename` | Output filename. |

---

### `inline void export_fetch_to_json(const std::vector<FetchContentInfo>& deps, const std::string& filename) `

> Exports FetchContent info to a JSON file.

| Parameter | Description |
| --- | --- |
| `deps` | List of FetchContent dependencies. |
| `filename` | Output filename. |

---

