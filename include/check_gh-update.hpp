/**
 * SPDX-FileComment: GitHub Release Checker Utility
 * SPDX-FileType: SOURCE
 * SPDX-FileContributor: ZHENG Robert
 * SPDX-FileCopyrightText: 2026 ZHENG Robert
 * SPDX-License-Identifier: MIT
 *
 * @file check_gh-update.hpp
 * @brief GitHub release checker utility with semantic versioning support.
 * @version 1.0.0
 * @date 2026-02-22
 *
 * @author ZHENG Robert (robert@hase-zheng.net)
 * @copyright Copyright (c) 2026 ZHENG Robert
 *
 * @license MIT License
 */

#pragma once
#include <string>
#include <string_view>
#include <regex>
#include <stdexcept>
#include <future>
#include <curl/curl.h>
#include <nlohmann/json.hpp>

/**
 * @namespace ghupdate
 * @brief Namespace for GitHub update checking utilities.
 */
namespace ghupdate {

/**
 * @struct SemVer
 * @brief Semantic versioning structure (major.minor.patch).
 */
struct SemVer {
    int major = 0;  ///< Major version component
    int minor = 0;  ///< Minor version component
    int patch = 0;  ///< Patch version component

    /**
     * @brief Parses a semantic version string.
     *
     * @param v Version string to parse.
     * @return Parsed SemVer structure.
     * @throws std::runtime_error if version format is invalid.
     */
    static SemVer parse(std::string_view v) {
        std::regex re(R"(v?(\d+)\.(\d+)(?:\.(\d+))?)");
        std::cmatch m;

        if (!std::regex_search(v.begin(), v.end(), m, re))
            throw std::runtime_error("Invalid SemVer: " + std::string(v));

        SemVer sv;
        sv.major = std::stoi(m[1]);
        sv.minor = std::stoi(m[2]);
        sv.patch = m[3].matched ? std::stoi(m[3]) : 0;
        return sv;
    }

    /**
     * @brief Three-way comparison operator for semantic version comparison.
     */
    auto operator<=>(const SemVer&) const = default;
};

/**
 * @brief CURL write callback for HTTP response buffering.
 *
 * @param contents Pointer to received data.
 * @param size Size of each element.
 * @param nmemb Number of elements.
 * @param userp User pointer (std::string* to accumulate data).
 * @return Total bytes written.
 */
static size_t write_callback(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t total = size * nmemb;
    static_cast<std::string*>(userp)->append((char*)contents, total);
    return total;
}

/**
 * @brief Performs an HTTP GET request using libcurl.
 *
 * @param url The URL to request.
 * @return Response body as std::string.
 * @throws std::runtime_error on network error.
 */
inline std::string http_get(std::string_view url) {
    CURL* curl = curl_easy_init();
    if (!curl) throw std::runtime_error("curl init failed");

    std::string buffer;

    curl_easy_setopt(curl, CURLOPT_URL, url.data());
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "C++23-gh-update-checker");
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);

    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK)
        throw std::runtime_error("HTTP request failed");

    return buffer;
}

/**
 * @brief Converts GitHub repository URLs to GitHub API URLs.
 *
 * @param url GitHub repository URL.
 * @return GitHub API URL for fetching releases.
 */
inline std::string to_github_api_url(std::string_view url) {
    if (url.find("api.github.com") != std::string::npos)
        return std::string(url);

    std::regex re(R"(https://github\.com/([^/]+)/([^/]+))");
    std::cmatch m;

    if (!std::regex_search(url.begin(), url.end(), m, re))
        throw std::runtime_error("Invalid GitHub URL: " + std::string(url));

    std::string owner = m[1];
    std::string repo  = m[2];

    if (repo.ends_with(".git"))
        repo = repo.substr(0, repo.size() - 4);

    return "https://api.github.com/repos/" + owner + "/" + repo + "/releases/latest";
}

/**
 * @struct UpdateInfo
 * @brief Result of a GitHub update check.
 */
struct UpdateInfo {
    bool hasUpdate;              ///< true if remote version > local version
    std::string latestVersion;   ///< Latest release tag from GitHub
};

/**
 * @brief Checks for updates on a GitHub repository (synchronous).
 *
 * @param repoUrl GitHub repository URL or API URL.
 * @param localVersion Local version string.
 * @return UpdateInfo structure containing update status.
 */
inline UpdateInfo check_github_update(
    std::string_view repoUrl,
    std::string_view localVersion
) {
    std::string apiUrl = to_github_api_url(repoUrl);

    std::string jsonText = http_get(apiUrl);
    auto json = nlohmann::json::parse(jsonText);

    if (!json.contains("tag_name") || !json["tag_name"].is_string()) {
        if (json.contains("message") && json["message"].is_string()) {
            throw std::runtime_error("GitHub API error: " + json["message"].get<std::string>());
        }
        throw std::runtime_error("GitHub API returned no valid tag_name");
    }

    std::string latest = json["tag_name"].get<std::string>();

    SemVer local = SemVer::parse(localVersion);
    SemVer remote = SemVer::parse(latest);

    return { remote > local, latest };
}

/**
 * @brief Checks for updates on a GitHub repository (asynchronous).
 *
 * @param repoUrl GitHub repository URL.
 * @param localVersion Local version string.
 * @return std::future resolving to UpdateInfo.
 */
inline std::future<UpdateInfo> check_github_update_async(
    std::string repoUrl,
    std::string localVersion
) {
    return std::async(std::launch::async, [repoUrl, localVersion] {
        return check_github_update(repoUrl, localVersion);
    });
}

} // namespace ghupdate
