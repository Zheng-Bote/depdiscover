# conan_parser.hpp

## File Header Information

| Field | Value |
| :--- | :--- |
| **SPDX Comment** | Conan Parser for Dependency Tracker |
| **SPDX Type** | SOURCE |
| **Contributor** | ZHENG Robert |
| **License ID** | MIT |
| **File** | `conan_parser.hpp` |
| **Description** | Parses conanfile.txt to identify dependencies. |
| **Version** | 1.0.0 |
| **Date** | 2026-02-18 |
| **Author** | ZHENG Robert (robert@hase-zheng.net) |
| **Copyright** | Copyright (c) 2026 ZHENG Robert |
| **License** | MIT License |

<!-- START doctoc generated TOC please keep comment here to allow auto update -->
<!-- DON'T EDIT THIS SECTION, INSTEAD RE-RUN doctoc TO UPDATE -->
**Table of Contents**

- [API Documentation](#api-documentation)
  - [`inline std::vector<Dependency>`](#inline-stdvectordependency)

<!-- END doctoc generated TOC please keep comment here to allow auto update -->

## API Documentation

### `inline std::vector<Dependency>`

> Parses a conanfile.txt to extract dependencies.

| Parameter | Description |
| --- | --- |
| `path` | The path to the conanfile.txt. |

**Returns:** std::vector<Dependency> A list of extracted dependencies.

---

