cmake_minimum_required(VERSION 3.23)

project(depdiscover 
          VERSION 1.5.2
          DESCRIPTION "Dependency Tracker for C++ Projects"

          HOMEPAGE_URL "https://github.com/Zheng-Bote/depdiscover"
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

# --- macOS RPATH Settings ---
if(APPLE)
    # Ensure @rpath is used in Mach-O headers
    set(CMAKE_MACOSX_RPATH ON)
    # Add @executable_path/../lib and CMAKE_INSTALL_PREFIX/lib to the search path for libraries
    # This allows the binary to find libraries if they are installed alongside it in a standard layout
    set(CMAKE_INSTALL_RPATH "@executable_path/../lib;${CMAKE_INSTALL_PREFIX}/lib")
    # Add paths to libraries outside the build tree to the install RPATH
    set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
endif()

# --- Find dependencies ---
# Prioritize Conan or system packages
find_package(nlohmann_json REQUIRED)
find_package(CURL REQUIRED)

# --- 1. Dependencies via FetchContent ---
set(FETCHCONTENT_TRY_FIND_PACKAGE_MODE ALWAYS)
include(FetchContent)

FetchContent_Declare(
  gh_update_checker
  GIT_REPOSITORY https://github.com/Zheng-Bote/gh-update-checker.git
  GIT_TAG        v1.0.6
)

FetchContent_MakeAvailable(gh_update_checker)

# Ensure gh_update_checker::gh_update_checker is available (for both FetchContent and find_package)
if(TARGET gh_update_checker AND NOT TARGET gh_update_checker::gh_update_checker)
    add_library(gh_update_checker::gh_update_checker ALIAS gh_update_checker)
endif()

add_executable(depdiscover src/main.cpp)

target_include_directories(depdiscover PRIVATE "${CMAKE_SOURCE_DIR}/include")
target_link_libraries(depdiscover PRIVATE 
    nlohmann_json::nlohmann_json
    gh_update_checker::gh_update_checker
    CURL::libcurl
)

if(MSVC)
    target_compile_options(depdiscover PRIVATE /W4)
else()
    target_compile_options(depdiscover PRIVATE -Wall -Wextra -Wpedantic)
endif()

# --- 4. Installation ---
include(GNUInstallDirs)
install(TARGETS depdiscover
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

# --- 5. Custom Generators (Original Code) ---

# CMake File API Query (Datei erzeugen)
set(QUERY_DIR "${CMAKE_BINARY_DIR}/.cmake/api/v1/query/client-project")
file(MAKE_DIRECTORY "${QUERY_DIR}")
file(WRITE "${QUERY_DIR}/codemodel-v2" "") 

# libs.txt Generierung (Timing-sicher via GENERATE)
file(GENERATE OUTPUT "${CMAKE_BINARY_DIR}/libs.txt" 
     CONTENT "$<TARGET_PROPERTY:depdiscover,LINK_LIBRARIES>")
