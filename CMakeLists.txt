cmake_minimum_required(VERSION 3.23)

project(depdiscover 
          VERSION 1.1.0
          DESCRIPTION "Dependency Tracker for C++ Projects"
          HOMEPAGE_URL "https://github.com/zb-bamboo/Dependency_Tracker_2"
          LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- Configure (für rz_config.hpp) ---
set(EXECUTABLE_NAME "${PROJECT_NAME}")
set(PROG_LONGNAME "Native C++ Dependency Scanner & SBOM Generator")
set(PROG_CREATED "2026")
set(PROG_AUTHOR "ZHENG Bote")
set(PROG_ORGANIZATION_NAME "ZHENG Robert")
set(PROG_ORGANIZATION_DOMAIN "net.hase-zheng")
# create rz_config.hpp
add_subdirectory(configure) 

# --- 1. Dependencies via FetchContent ---
include(FetchContent)

FetchContent_Declare(
  json
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG        v3.11.3
)
FetchContent_MakeAvailable(json)


add_executable(depdiscover src/main.cpp)

target_include_directories(depdiscover PRIVATE "${CMAKE_SOURCE_DIR}/include")
target_link_libraries(depdiscover PRIVATE nlohmann_json::nlohmann_json)

if(MSVC)
    target_compile_options(depdiscover PRIVATE /W4)
else()
    target_compile_options(depdiscover PRIVATE -Wall -Wextra -Wpedantic)
endif()

# --- 3. Custom Generators (Original Code) ---

# CMake File API Query (Datei erzeugen)
set(QUERY_DIR "${CMAKE_BINARY_DIR}/.cmake/api/v1/query/client-project")
file(MAKE_DIRECTORY "${QUERY_DIR}")
file(WRITE "${QUERY_DIR}/codemodel-v2" "") 

# libs.txt Generierung (Timing-sicher via GENERATE)
file(GENERATE OUTPUT "${CMAKE_BINARY_DIR}/libs.txt" 
     CONTENT "$<TARGET_PROPERTY:depdiscover,LINK_LIBRARIES>")