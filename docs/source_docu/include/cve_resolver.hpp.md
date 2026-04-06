# cve_resolver.hpp

## File Header Information

| Field | Value |
| :--- | :--- |
| **SPDX Comment** | CVE Resolver and OSV Query Tool |
| **SPDX Type** | SOURCE |
| **Contributor** | ZHENG Robert |
| **License ID** | MIT |
| **File** | `cve_resolver.hpp` |
| **Description** | Queries OSV.dev for CVEs associated with packages. |
| **Version** | 1.3.1 |
| **Date** | 2026-02-18 |
| **Author** | ZHENG Robert (robert@hase-zheng.net) |
| **Copyright** | Copyright (c) 2026 ZHENG Robert |
| **License** | MIT License |

<!-- START doctoc generated TOC please keep comment here to allow auto update -->
<!-- END doctoc generated TOC please keep comment here to allow auto update -->

## API Documentation

### `static size_t curl_write_callback(void *contents, size_t size, size_t nmemb,`

> CURL write callback for HTTP response buffering.

---

### `inline std::string get_current_date() `

> Returns the current date as a string in YYYY-MM-DD format.

**Returns:** std::string The formatted date.

---

### `inline std::string perform_curl_post(const std::string &url,`

> Performs a secure curl POST request using libcurl.

| Parameter | Description |
| --- | --- |
| `url` | The target URL. |
| `json_payload` | The JSON payload to send. |

**Returns:** std::string The response from the server.

---

### `inline std::vector<CVE> query_cves(const std::string &name,`

> Queries OSV.dev for vulnerabilities related to a package and version.

| Parameter | Description |
| --- | --- |
| `name` | The name of the package. |
| `version` | The version of the package. |
| `ecosystem` | The OSV ecosystem (default: "Debian"). |

**Returns:** std::vector<CVE> A list of vulnerabilities found.

---

