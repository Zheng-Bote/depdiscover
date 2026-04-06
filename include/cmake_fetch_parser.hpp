/**
 * SPDX-FileComment: CMake FetchContent Parser
 * SPDX-FileType: SOURCE
 * SPDX-FileContributor: ZHENG Robert
 * SPDX-FileCopyrightText: 2026 ZHENG Robert
 * SPDX-License-Identifier: MIT
 *
 * @file cmake_fetch_parser.hpp
 * @brief Parses CMakeLists.txt to extract FetchContent_Declare information and export to CSV/JSON.
 * @version 1.1.0
 * @date 2026-04-06
 *
 * @author ZHENG Robert (robert@hase-zheng.net)
 * @copyright Copyright (c) 2026 ZHENG Robert
 *
 * @license MIT License
 */
#pragma once
#include "types.hpp"
#include <fstream>
#include <regex>
#include <string>
#include <vector>
#include <map>
#include <nlohmann/json.hpp>

namespace depdiscover {

/**
 * @brief Structure to hold FetchContent dependency information.
 */
struct FetchContentInfo {
    std::string name;
    std::string url;
    std::string version;
};

/**
 * @brief Extracts FetchContent_Declare blocks from a CMake file.
 *
 * @param cmake_path Path to the CMakeLists.txt file.
 * @return std::vector<FetchContentInfo> List of extracted dependency info.
 */
inline std::vector<FetchContentInfo> parse_cmake_fetch_content(const std::string &cmake_path) {
    std::vector<FetchContentInfo> fetch_deps;
    std::ifstream f(cmake_path);
    if (!f.is_open()) return fetch_deps;

    std::string content((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
    
    // Regex to find FetchContent_Declare block and capture its content
    std::regex re_declare(R"(FetchContent_Declare\s*\(\s*([A-Za-z0-9_.-]+)\s+([\s\S]*?)\s*\))", std::regex::icase);
    
    auto words_begin = std::sregex_iterator(content.begin(), content.end(), re_declare);
    auto words_end = std::sregex_iterator();

    for (std::sregex_iterator i = words_begin; i != words_end; ++i) {
        std::smatch match = *i;
        FetchContentInfo info;
        info.name = match[1].str();
        std::string block_content = match[2].str();

        // Inside the block, look for GIT_REPOSITORY
        std::regex re_repo(R"(GIT_REPOSITORY\s+([^\s\)\n\r]+))", std::regex::icase);
        std::smatch repo_match;
        if (std::regex_search(block_content, repo_match, re_repo)) {
            info.url = repo_match[1].str();
            if (info.url.size() >= 2 && info.url.front() == '"' && info.url.back() == '"') {
                info.url = info.url.substr(1, info.url.size() - 2);
            }
        }

        // Inside the block, look for GIT_TAG
        std::regex re_tag(R"(GIT_TAG\s+([^\s\)\n\r]+))", std::regex::icase);
        std::smatch tag_match;
        if (std::regex_search(block_content, tag_match, re_tag)) {
            info.version = tag_match[1].str();
            if (info.version.size() >= 2 && info.version.front() == '"' && info.version.back() == '"') {
                info.version = info.version.substr(1, info.version.size() - 2);
            }
        }
        
        if (!info.name.empty()) {
            fetch_deps.push_back(info);
        }
    }

    return fetch_deps;
}

/**
 * @brief Exports FetchContent info to a CSV file.
 * 
 * @param deps List of FetchContent dependencies.
 * @param filename Output filename.
 */
inline void export_fetch_to_csv(const std::vector<FetchContentInfo>& deps, const std::string& filename) {
    std::ofstream f(filename);
    if (!f.is_open()) return;
    
    for (const auto& d : deps) {
        f << d.name << "," << d.url << "," << d.version << "\n";
    }
}

/**
 * @brief Exports FetchContent info to a JSON file.
 * 
 * @param deps List of FetchContent dependencies.
 * @param filename Output filename.
 */
inline void export_fetch_to_json(const std::vector<FetchContentInfo>& deps, const std::string& filename) {
    nlohmann::json j = nlohmann::json::array();
    for (const auto& d : deps) {
        nlohmann::json item;
        item["name"] = d.name;
        item["url"] = d.url;
        item["version"] = d.version;
        j.push_back(item);
    }
    
    std::ofstream f(filename);
    if (f.is_open()) {
        f << j.dump(2);
    }
}

} // namespace depdiscover
