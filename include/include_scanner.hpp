#pragma once
#include <regex>
#include <string>
#include <vector>

namespace depdiscover {

inline std::vector<std::string> extract_include_paths(const std::string &cmd) {
  std::vector<std::string> out;
  // Fix: \\s* erlaubt Leerzeichen (z.B. "-I /usr/include")
  // ([^\\s]+) fängt alles bis zum nächsten Whitespace
  std::regex re("-I\\s*([^\\s]+)");
  std::smatch m;

  auto begin = cmd.cbegin();
  auto end = cmd.cend();

  while (std::regex_search(begin, end, m, re)) {
    out.push_back(m[1]);
    begin = m.suffix().first;
  }

  return out;
}

inline std::vector<std::string> extract_libraries(const std::string &cmd) {
  std::vector<std::string> out;
  // Gleicher Fix für Libraries (-l lib vs -lLIB)
  std::regex re("-l\\s*([^\\s]+)");
  std::smatch m;

  auto begin = cmd.cbegin();
  auto end = cmd.cend();

  while (std::regex_search(begin, end, m, re)) {
    out.push_back(m[1]);
    begin = m.suffix().first;
  }

  return out;
}

} // namespace depdiscover