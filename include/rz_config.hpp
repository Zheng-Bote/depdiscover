/**
 * @file rz_config.hpp.in
 * @author ZHENG Robert (robert@hase-zheng.net)
 * @brief configuration
 * @version 2.1.0
 * @date 2026-01-01
 *
 * @copyright Copyright (c) 2025 ZHENG Robert
 *
 * SPDX-License-Identifier: MIT
 */
#pragma once

#include <string_view>

namespace rz {
namespace config {
constexpr std::string_view PROJECT_NAME = "depdiscover";
constexpr std::string_view PROG_LONGNAME = "Native C++ Dependency Scanner & SBOM Generator";
constexpr std::string_view PROJECT_DESCRIPTION = "Dependency Tracker for C++ Projects";

constexpr std::string_view EXECUTABLE_NAME = "depdiscover";

constexpr std::string_view VERSION = "1.1.0";
constexpr std::int32_t PROJECT_VERSION_MAJOR { 1 };
constexpr std::int32_t PROJECT_VERSION_MINOR { 1 };
constexpr std::int32_t PROJECT_VERSION_PATCH { 0 };

constexpr std::string_view PROJECT_HOMEPAGE_URL = "https://github.com/zb-bamboo/Dependency_Tracker_2";
constexpr std::string_view AUTHOR = "ZHENG Bote";
constexpr std::string_view CREATED_YEAR = "2026";
constexpr std::string_view ORGANIZATION = "ZHENG Robert";
constexpr std::string_view DOMAIN = "net.hase-zheng";

constexpr std::string_view CMAKE_CXX_STANDARD = "c++23";
constexpr std::string_view CMAKE_CXX_COMPILER =
    "GNU 15.2.0";
constexpr std::string_view QT_VERSION_BUILD = "";
} // namespace config
} // namespace rz
