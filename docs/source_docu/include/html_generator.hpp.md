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
<!-- END doctoc generated TOC please keep comment here to allow auto update -->

## API Documentation

### `inline double extract_score_for_html(const std::string &severity_str) `

> Helper function to extract a CVSS score from a severity string for HTML display.

| Parameter | Description |
| --- | --- |
| `severity_str` | The severity string. |

**Returns:** double The extracted score.

---

### `inline void generate_html_report(const nlohmann::json &root,`

> Generates an HTML report from the internal JSON representation.

| Parameter | Description |
| --- | --- |
| `root` | The root of the internal JSON data. |
| `filepath` | The path where the report will be saved. |

---

