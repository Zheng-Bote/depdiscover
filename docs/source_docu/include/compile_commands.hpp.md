# compile_commands.hpp

## File Header Information

| Field | Value |
| :--- | :--- |
| **SPDX Comment** | Compile Commands JSON Parser |
| **SPDX Type** | SOURCE |
| **Contributor** | ZHENG Robert |
| **License ID** | MIT |
| **File** | `compile_commands.hpp` |
| **Description** | Parses compile_commands.json files to extract build information. |
| **Version** | 1.0.0 |
| **Date** | 2026-02-18 |
| **Author** | ZHENG Robert (robert@hase-zheng.net) |
| **Copyright** | Copyright (c) 2026 ZHENG Robert |
| **License** | MIT License |

<!-- START doctoc generated TOC please keep comment here to allow auto update -->
<!-- DON'T EDIT THIS SECTION, INSTEAD RE-RUN doctoc TO UPDATE -->
**Table of Contents**

- [API Documentation](#api-documentation)
  - [`struct CompileCommand `](#struct-compilecommand-)
  - [`inline std::vector<CompileCommand>`](#inline-stdvectorcompilecommand)

<!-- END doctoc generated TOC please keep comment here to allow auto update -->

## API Documentation

### `struct CompileCommand `

> Represents a single entry in a compile_commands.json file.

---

### `inline std::vector<CompileCommand>`

> Loads and parses a compile_commands.json file.

| Parameter | Description |
| --- | --- |
| `path` | The path to the compile_commands.json file. |

**Returns:** std::vector<CompileCommand> A list of compile commands.

---

