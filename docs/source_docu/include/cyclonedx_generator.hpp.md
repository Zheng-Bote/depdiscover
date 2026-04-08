# cyclonedx_generator.hpp

## File Header Information

| Field | Value |
| :--- | :--- |
| **SPDX Comment** | CycloneDX Generator |
| **SPDX Type** | SOURCE |
| **Contributor** | ZHENG Robert |
| **License ID** | MIT |
| **File** | `cyclonedx_generator.hpp` |
| **Description** | Generates a valid CycloneDX 1.4 SBOM from internal JSON data. |
| **Version** | 1.0.0 |
| **Date** | 2026-02-22 |
| **Author** | ZHENG Robert (robert@hase-zheng.net) |
| **Copyright** | Copyright (c) 2026 ZHENG Robert |
| **License** | MIT License |

<!-- START doctoc generated TOC please keep comment here to allow auto update -->
<!-- DON'T EDIT THIS SECTION, INSTEAD RE-RUN doctoc TO UPDATE -->
**Table of Contents**

- [API Documentation](#api-documentation)
  - [`inline std::string get_iso8601_timestamp() `](#inline-stdstring-get_iso8601_timestamp-)
  - [`inline std::string generate_uuid_v4() `](#inline-stdstring-generate_uuid_v4-)
  - [`inline void generate_cyclonedx_report(const json &internal_root,`](#inline-void-generate_cyclonedx_reportconst-json-internal_root)

<!-- END doctoc generated TOC please keep comment here to allow auto update -->

## API Documentation

### `inline std::string get_iso8601_timestamp() `

> Returns an ISO-8601 timestamp for CycloneDX (YYYY-MM-DDThh:mm:ssZ).

**Returns:** std::string The formatted timestamp.

---

### `inline std::string generate_uuid_v4() `

> Generates a pseudo-UUIDv4 for the SBOM serial number.

**Returns:** std::string The generated UUID.

---

### `inline void generate_cyclonedx_report(const json &internal_root,`

> Generates a CycloneDX 1.4 report from the internal JSON representation.

| Parameter | Description |
| --- | --- |
| `internal_root` | The root of the internal JSON data. |
| `filepath` | The path where the report will be saved. |

---

