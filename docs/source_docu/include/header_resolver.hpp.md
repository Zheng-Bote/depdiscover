# header_resolver.hpp

## File Header Information

| Field | Value |
| :--- | :--- |
| **SPDX Comment** | Header File Resolver |
| **SPDX Type** | SOURCE |
| **Contributor** | ZHENG Robert |
| **License ID** | MIT |
| **File** | `header_resolver.hpp` |
| **Description** | Scans include directives and resolves them to absolute paths. |
| **Version** | 1.0.0 |
| **Date** | 2026-02-18 |
| **Author** | ZHENG Robert (robert@hase-zheng.net) |
| **Copyright** | Copyright (c) 2026 ZHENG Robert |
| **License** | MIT License |

<!-- START doctoc generated TOC please keep comment here to allow auto update -->
<!-- END doctoc generated TOC please keep comment here to allow auto update -->

## API Documentation

### `inline std::vector<std::string> scan_includes(const std::string &source_file) `

> Scans a source file for `#include` directives.

| Parameter | Description |
| --- | --- |
| `source_file` | The path to the source file. |

**Returns:** std::vector<std::string> A list of included header names.

---

### `inline std::string resolve_header(const std::string &header_name,`

> Resolves a header filename to an absolute path.

| Parameter | Description |
| --- | --- |
| `header_name` | The name of the header file. |
| `include_paths` | A list of include directories. |
| `work_dir` | The working directory where the compiler was executed (for relative include paths). |

**Returns:** std::string The absolute path to the header, or empty if not found.

---

