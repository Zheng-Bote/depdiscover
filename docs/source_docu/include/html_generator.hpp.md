# html_generator.hpp

## File Header Information

| Field | Value |
| :--- | :--- |
| **SPDX Comment** | HTML Report Generator |
| **SPDX Type** | SOURCE |
| **Contributor** | ZHENG Robert |
| **License ID** | MIT |
| **File** | `html_generator.hpp` |
| **Description** | Generates a user-friendly HTML report from the SBOM JSON data. |
| **Version** | 1.4.0 |
| **Date** | 2026-02-22 |
| **Author** | ZHENG Robert (robert@hase-zheng.net) |
| **Copyright** | Copyright (c) 2026 ZHENG Robert |
| **License** | MIT License |

<!-- START doctoc generated TOC please keep comment here to allow auto update -->
<!-- DON'T EDIT THIS SECTION, INSTEAD RE-RUN doctoc TO UPDATE -->
**Table of Contents**

- [API Documentation](#api-documentation)
  - [`inline void generate_html_report(const nlohmann::json &root,`](#inline-void-generate_html_reportconst-nlohmannjson-root)

<!-- END doctoc generated TOC please keep comment here to allow auto update -->

## API Documentation

### `inline void generate_html_report(const nlohmann::json &root,`

> Generates an HTML security report from the internal JSON representation.

| Parameter | Description |
| --- | --- |
| `root` | The root of the internal JSON data. |
| `filepath` | The path where the report will be saved. |

---

