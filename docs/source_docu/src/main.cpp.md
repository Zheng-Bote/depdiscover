# main.cpp

## File Header Information

| Field | Value |
| :--- | :--- |
| **SPDX Comment** | Main entry point for the Dependency Tracker application |
| **SPDX Type** | SOURCE |
| **Contributor** | ZHENG Robert |
| **License ID** | MIT |
| **File** | `main.cpp` |
| **Description** | Main entry point for the Dependency Tracker application. |
| **Version** | 1.4.0 |
| **Date** | 2026-04-06 |
| **Author** | ZHENG Robert (robert@hase-zheng.net) |
| **Copyright** | Copyright (c) 2026 ZHENG Robert |
| **License** | MIT License |

<!-- START doctoc generated TOC please keep comment here to allow auto update -->
<!-- DON'T EDIT THIS SECTION, INSTEAD RE-RUN doctoc TO UPDATE -->
**Table of Contents**

- [API Documentation](#api-documentation)
  - [`constexpr auto SCHEMA_VERSION = "1.2"`](#constexpr-auto-schema_version--12)
  - [`bool string_contains(const std::string &haystack, const std::string &needle) `](#bool-string_containsconst-stdstring-haystack-const-stdstring-needle-)
  - [`bool path_starts_with(const std::string &path, const std::string &prefix) `](#bool-path_starts_withconst-stdstring-path-const-stdstring-prefix-)
  - [`std::string get_platform_name() `](#stdstring-get_platform_name-)
  - [`bool fuzzy_match_header(const std::string &header_path,`](#bool-fuzzy_match_headerconst-stdstring-header_path)
  - [`bool fuzzy_match_lib(const std::string &lib_filename,`](#bool-fuzzy_match_libconst-stdstring-lib_filename)
  - [`void check_for_updates() `](#void-check_for_updates-)
  - [`void print_help(const char *program_name) `](#void-print_helpconst-char-program_name-)
  - [`int main(int argc, char **argv) `](#int-mainint-argc-char-argv-)

<!-- END doctoc generated TOC please keep comment here to allow auto update -->

## API Documentation

### `constexpr auto SCHEMA_VERSION = "1.2"`

> Current schema version for the generated JSON report.

---

### `bool string_contains(const std::string &haystack, const std::string &needle) `

> Checks if a string contains another string (case-insensitive).

| Parameter | Description |
| --- | --- |
| `haystack` | The string to search in. |
| `needle` | The string to search for. |

**Returns:** true If the needle is found in the haystack.

---

### `bool path_starts_with(const std::string &path, const std::string &prefix) `

> Checks if a path starts with a given prefix.

| Parameter | Description |
| --- | --- |
| `path` | The path to check. |
| `prefix` | The prefix to look for. |

**Returns:** true If the path starts with the prefix.

---

### `std::string get_platform_name() `

> Identifies the current OS platform.

**Returns:** std::string "Windows", "macOS", "Linux" or "Unknown".

---

### `bool fuzzy_match_header(const std::string &header_path,`

> Performs a fuzzy match between a header path and a package name.

| Parameter | Description |
| --- | --- |
| `header_path` | The path to the header file. |
| `pkg_name` | The name of the package. |

**Returns:** true If a match is likely.

---

### `bool fuzzy_match_lib(const std::string &lib_filename,`

> Performs a fuzzy match between a library filename and a package name.

| Parameter | Description |
| --- | --- |
| `lib_filename` | The filename of the library. |
| `pkg_name` | The name of the package. |

**Returns:** true If a match is likely.

---

### `void check_for_updates() `

> Checks for updates of depdiscover itself.

---

### `void print_help(const char *program_name) `

> Prints the help message to the standard error stream.

| Parameter | Description |
| --- | --- |
| `program_name` | The name of the executable. |

---

### `int main(int argc, char **argv) `

> Main entry point of the application.

| Parameter | Description |
| --- | --- |
| `argc` | Number of command-line arguments. |
| `argv` | Array of command-line arguments. |

**Returns:** int Exit code (0 for success, non-zero for failure).

---

