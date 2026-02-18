#pragma once
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

namespace depdiscover {

// CVE Struktur
struct CVE {
  std::string id;
  std::string summary;
  std::string severity;
  std::string fixed_version;
};

inline void to_json(nlohmann::json &j, const CVE &c) {
  j = nlohmann::json{{"id", c.id},
                     {"summary", c.summary},
                     {"severity", c.severity},
                     {"fixed_version", c.fixed_version}};
}

// Dependency Struktur (Erweitert)
struct Dependency {
  std::string name;
  std::string version;
  std::string type;   // "vcpkg", "conan", "system"
  std::string source; // "manifest", "pkgconfig"

  std::vector<std::string> headers;
  std::vector<std::string> libraries;

  // NEU: Metadaten
  std::vector<std::string> licenses;
  std::vector<CVE> cves;
};

inline void to_json(nlohmann::json &j, const Dependency &d) {
  j = nlohmann::json{{"name", d.name},
                     {"version", d.version},
                     {"type", d.type},
                     {"source", d.source.empty() ? "manifest" : d.source},
                     {"headers", d.headers},
                     {"libraries", d.libraries},
                     {"licenses", d.licenses},
                     {"cves", d.cves}};
}

} // namespace depdiscover