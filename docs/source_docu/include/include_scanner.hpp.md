# include_scanner.hpp

## File Header Information

| Field | Value |
| :--- | :--- |
| **SPDX Comment** | Include Path and Library Scanner |
| **SPDX Type** | SOURCE |
| **Contributor** | ZHENG Robert |
| **License ID** | MIT |
| **File** | `include_scanner.hpp` |
| **Description** | Scans compiler flags for include paths and library names. |
| **Version** | 1.0.0 |
| **Date** | 2026-02-18 |
| **Author** | ZHENG Robert (robert@hase-zheng.net) |
| **Copyright** | Copyright (c) 2026 ZHENG Robert |
| **License** | MIT License |

<!-- START doctoc generated TOC please keep comment here to allow auto update -->
<!-- END doctoc generated TOC please keep comment here to allow auto update -->

## API Documentation

### `inline std::vector<std::string> extract_include_paths(const std::string &cmd) `

> Extracts include paths (-I flag) from a compiler command string.

| Parameter | Description |
| --- | --- |
| `cmd` | The compiler command string. |

**Returns:** std::vector<std::string> A list of extracted include paths.

---

### `inline std::vector<std::string> extract_libraries(const std::string &cmd) `

> Extracts library names (-l flag) from a compiler command string.

| Parameter | Description |
| --- | --- |
| `cmd` | The compiler command string. |

**Returns:** std::vector<std::string> A list of extracted library names.

---

