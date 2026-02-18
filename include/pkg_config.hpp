#pragma once
#include <array>
#include <cstdio>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

namespace depdiscover {

struct PkgInfo {
  bool found = false;
  std::string version;
  std::vector<std::string> include_paths;
  std::vector<std::string> lib_names;
};

class PkgConfig {
public:
  static PkgInfo query(const std::string &package_name) {
    PkgInfo info;

    // Version abrufen
    std::string ver = exec("pkg-config --modversion " + package_name);
    if (!ver.empty()) {
      // Newline am Ende entfernen
      while (!ver.empty() && (ver.back() == '\n' || ver.back() == '\r'))
        ver.pop_back();
      info.version = ver;
      info.found = true;
    } else {
      return info; // Paket nicht gefunden
    }

    // Include Pfade (-I)
    std::string cflags = exec("pkg-config --cflags-only-I " + package_name);
    info.include_paths = parse_flags(cflags, "-I");

    // Lib Namen (-l)
    std::string libs = exec("pkg-config --libs-only-l " + package_name);
    info.lib_names = parse_flags(libs, "-l");

    return info;
  }

private:
  static std::string exec(const std::string &cmd) {
    std::array<char, 256> buffer;
    std::string result;
    // stderr wegwerfen
    std::unique_ptr<FILE, decltype(&pclose)> pipe(
        popen((cmd + " 2>/dev/null").c_str(), "r"), pclose);
    if (!pipe)
      return "";
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
      result += buffer.data();
    }
    return result;
  }

  static std::vector<std::string> parse_flags(const std::string &input,
                                              const std::string &prefix) {
    std::vector<std::string> out;
    std::stringstream ss(input);
    std::string segment;

    // Splitte bei Whitespace
    while (ss >> segment) {
      if (segment.rfind(prefix, 0) == 0) { // Beginnt mit Prefix (z.B. -I)
        std::string val = segment.substr(prefix.length());
        if (!val.empty())
          out.push_back(val);
      }
    }
    return out;
  }
};

} // namespace depdiscover