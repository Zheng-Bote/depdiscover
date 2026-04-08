# check_gh-update.hpp

## File Header Information

| Field | Value |
| :--- | :--- |
| **SPDX Comment** | GitHub Release Checker Utility |
| **SPDX Type** | SOURCE |
| **Contributor** | ZHENG Robert |
| **License ID** | MIT |
| **File** | `check_gh-update.hpp` |
| **Description** | GitHub release checker utility with semantic versioning support. |
| **Version** | 1.0.0 |
| **Date** | 2026-02-22 |
| **Author** | ZHENG Robert (robert@hase-zheng.net) |
| **Copyright** | Copyright (c) 2026 ZHENG Robert |
| **License** | MIT License |

<!-- START doctoc generated TOC please keep comment here to allow auto update -->
<!-- DON'T EDIT THIS SECTION, INSTEAD RE-RUN doctoc TO UPDATE -->
**Table of Contents**

- [API Documentation](#api-documentation)
  - [`namespace ghupdate `](#namespace-ghupdate-)
  - [`struct SemVer `](#struct-semver-)
  - [`static SemVer parse(std::string_view v) `](#static-semver-parsestdstring_view-v-)
  - [`auto operator<=>(const SemVer&) const = default`](#auto-operatorconst-semver-const--default)
  - [`static size_t write_callback(void* contents, size_t size, size_t nmemb, void* userp) `](#static-size_t-write_callbackvoid-contents-size_t-size-size_t-nmemb-void-userp-)
  - [`inline std::string http_get(std::string_view url) `](#inline-stdstring-http_getstdstring_view-url-)
  - [`inline std::string to_github_api_url(std::string_view url) `](#inline-stdstring-to_github_api_urlstdstring_view-url-)
  - [`struct UpdateInfo `](#struct-updateinfo-)
  - [`inline UpdateInfo check_github_update(`](#inline-updateinfo-check_github_update)
  - [`inline std::future<UpdateInfo> check_github_update_async(`](#inline-stdfutureupdateinfo-check_github_update_async)

<!-- END doctoc generated TOC please keep comment here to allow auto update -->

## API Documentation

### `namespace ghupdate `

> Namespace for GitHub update checking utilities.

---

### `struct SemVer `

> Semantic versioning structure (major.minor.patch).

---

### `static SemVer parse(std::string_view v) `

> Parses a semantic version string.

| Parameter | Description |
| --- | --- |
| `v` | Version string to parse. |

**Returns:** Parsed SemVer structure.

---

### `auto operator<=>(const SemVer&) const = default`

> Three-way comparison operator for semantic version comparison.

---

### `static size_t write_callback(void* contents, size_t size, size_t nmemb, void* userp) `

> CURL write callback for HTTP response buffering.

| Parameter | Description |
| --- | --- |
| `contents` | Pointer to received data. |
| `size` | Size of each element. |
| `nmemb` | Number of elements. |
| `userp` | User pointer (std::string* to accumulate data). |

**Returns:** Total bytes written.

---

### `inline std::string http_get(std::string_view url) `

> Performs an HTTP GET request using libcurl.

| Parameter | Description |
| --- | --- |
| `url` | The URL to request. |

**Returns:** Response body as std::string.

---

### `inline std::string to_github_api_url(std::string_view url) `

> Converts GitHub repository URLs to GitHub API URLs.

| Parameter | Description |
| --- | --- |
| `url` | GitHub repository URL. |

**Returns:** GitHub API URL for fetching releases.

---

### `struct UpdateInfo `

> Result of a GitHub update check.

---

### `inline UpdateInfo check_github_update(`

> Checks for updates on a GitHub repository (synchronous).

| Parameter | Description |
| --- | --- |
| `repoUrl` | GitHub repository URL or API URL. |
| `localVersion` | Local version string. |

**Returns:** UpdateInfo structure containing update status.

---

### `inline std::future<UpdateInfo> check_github_update_async(`

> Checks for updates on a GitHub repository (asynchronous).

| Parameter | Description |
| --- | --- |
| `repoUrl` | GitHub repository URL. |
| `localVersion` | Local version string. |

**Returns:** std::future resolving to UpdateInfo.

---

