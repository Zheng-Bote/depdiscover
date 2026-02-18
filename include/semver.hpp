#pragma once
#include <regex>
#include <string>

namespace depdiscover {

inline std::string clean_version(const std::string &raw_version) {
  if (raw_version.empty())
    return "";

  // Regex Erklärung:
  // (\d+)       -> Hauptversion (Zahl)
  // \.          -> Punkt
  // (\d+)       -> Nebenversion (Zahl)
  // (?:\.(\d+))? -> Optional: Punkt und Patch-Version
  //
  // std::regex_search sucht den ERSTEN Treffer im String (ignoriert
  // Prefix/Suffix)
  static const std::regex re(R"((\d+)\.(\d+)(?:\.(\d+))?)");
  std::smatch m;

  if (std::regex_search(raw_version, m, re)) {
    // Gibt den kompletten Match zurück (z.B. "3.11.2" aus "v3.11.2")
    return m[0].str();
  }

  // Fallback: Wenn kein Versions-Muster erkannt wird (z.B. "latest", "system"),
  // geben wir den Original-String zurück.
  return raw_version;
}

} // namespace depdiscover