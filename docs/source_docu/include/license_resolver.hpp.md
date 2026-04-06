# license_resolver.hpp

## File Header Information

| Field | Value |
| :--- | :--- |
| **SPDX Comment** | License Resolver |
| **SPDX Type** | SOURCE |
| **Contributor** | ZHENG Robert |
| **License ID** | MIT |
| **File** | `license_resolver.hpp` |
| **Description** | Resolves licenses for dependencies using heuristics and file scanning. |
| **Version** | 1.0.0 |
| **Date** | 2026-02-18 |
| **Author** | ZHENG Robert (robert@hase-zheng.net) |
| **Copyright** | Copyright (c) 2026 ZHENG Robert |
| **License** | MIT License |

<!-- START doctoc generated TOC please keep comment here to allow auto update -->
<!-- DON'T EDIT THIS SECTION, INSTEAD RE-RUN doctoc TO UPDATE -->
**Table of Contents**

- [API Documentation](#api-documentation)
  - [`inline std::string guess_license_from_content(const fs::path &path) `](#inline-stdstring-guess_license_from_contentconst-fspath-path-)
  - [`inline std::vector<std::string>`](#inline-stdvectorstdstring)

<!-- END doctoc generated TOC please keep comment here to allow auto update -->

## API Documentation

### `inline std::string guess_license_from_content(const fs::path &path) `

> Guesses the license type from a file's content.

| Parameter | Description |
| --- | --- |
| `path` | The path to the file (e.g., LICENSE). |

**Returns:** std::string The matched license (e.g., "MIT"), or "See file: ..." if ambiguous.

---

### `inline std::vector<std::string>`

> Resolves licenses for a package.

| Parameter | Description |
| --- | --- |
| `package_name` | The name of the package. |
| `header_files` | A list of associated header files (used for locating license files). |

**Returns:** std::vector<std::string> A list of resolved licenses.

---

