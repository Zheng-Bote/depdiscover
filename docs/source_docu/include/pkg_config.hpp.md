# pkg_config.hpp

## File Header Information

| Field | Value |
| :--- | :--- |
| **SPDX Comment** | Pkg-Config Wrapper |
| **SPDX Type** | SOURCE |
| **Contributor** | ZHENG Robert |
| **License ID** | MIT |
| **File** | `pkg_config.hpp` |
| **Description** | Wrapper class for querying pkg-config. |
| **Version** | 1.0.0 |
| **Date** | 2026-02-18 |
| **Author** | ZHENG Robert (robert@hase-zheng.net) |
| **Copyright** | Copyright (c) 2026 ZHENG Robert |
| **License** | MIT License |

<!-- START doctoc generated TOC please keep comment here to allow auto update -->
<!-- DON'T EDIT THIS SECTION, INSTEAD RE-RUN doctoc TO UPDATE -->
**Table of Contents**

- [API Documentation](#api-documentation)
  - [`struct PkgInfo `](#struct-pkginfo-)
  - [`class PkgConfig `](#class-pkgconfig-)
  - [`static PkgInfo query(const std::string &package_name) `](#static-pkginfo-queryconst-stdstring-package_name-)
  - [`static std::string exec(const std::string &cmd) `](#static-stdstring-execconst-stdstring-cmd-)
  - [`static std::vector<std::string> parse_flags(const std::string &input,`](#static-stdvectorstdstring-parse_flagsconst-stdstring-input)

<!-- END doctoc generated TOC please keep comment here to allow auto update -->

## API Documentation

### `struct PkgInfo `

> Holds information returned by pkg-config.

---

### `class PkgConfig `

> Static class for querying pkg-config.

---

### `static PkgInfo query(const std::string &package_name) `

> Queries pkg-config for a specific package.

| Parameter | Description |
| --- | --- |
| `package_name` | The name of the package. |

**Returns:** PkgInfo The result of the query.

---

### `static std::string exec(const std::string &cmd) `

> Executes a shell command and returns the output.

| Parameter | Description |
| --- | --- |
| `cmd` | The command to execute. |

**Returns:** std::string The command's stdout.

---

### `static std::vector<std::string> parse_flags(const std::string &input,`

> Parses a string of compiler flags.

| Parameter | Description |
| --- | --- |
| `input` | The flags string. |
| `prefix` | The prefix to look for (e.g., "-I"). |

**Returns:** std::vector<std::string> The extracted values.

---

