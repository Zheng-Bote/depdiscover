/**
 * SPDX-FileComment: Compile Commands JSON Parser
 * SPDX-FileType: SOURCE
 * SPDX-FileContributor: ZHENG Robert
 * SPDX-FileCopyrightText: 2026 ZHENG Robert
 * SPDX-License-Identifier: MIT
 *
 * @file compile_commands.hpp
 * @brief Parses compile_commands.json files to extract build information.
 * @version 1.0.0
 * @date 2026-02-18
 *
 * @author ZHENG Robert (robert@hase-zheng.net)
 * @copyright Copyright (c) 2026 ZHENG Robert
 *
 * @license MIT License
 */
#pragma once
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <stdexcept>
#include <string>
#include <vector>

namespace depdiscover {

/**
 * @brief Represents a single entry in a compile_commands.json file.
 */
struct CompileCommand {
  std::string file;
  std::string command;
  std::string directory; // NEU: Arbeitsverzeichnis
};

/**
 * @brief Loads and parses a compile_commands.json file.
 *
 * @param path The path to the compile_commands.json file.
 * @return std::vector<CompileCommand> A list of compile commands.
 * @throws std::runtime_error If the file cannot be opened or parsed.
 */
inline std::vector<CompileCommand>
load_compile_commands(const std::string &path) {
  std::ifstream f(path);
  if (!f)
    throw std::runtime_error("compile_commands.json not found at: " + path);

  nlohmann::json doc;
  try {
    f >> doc;
  } catch (const nlohmann::json::parse_error &e) {
    throw std::runtime_error(std::string("JSON parse error: ") + e.what());
  }

  if (!doc.is_array())
    throw std::runtime_error("compile_commands.json: expected top-level array");

  std::vector<CompileCommand> out;
  out.reserve(doc.size());

  for (const auto &entry : doc) {
    std::string file;
    std::string command;
    std::string directory;

    if (entry.contains("file"))
      file = entry["file"].get<std::string>();
    else
      continue;

    if (entry.contains("directory"))
      directory = entry["directory"].get<std::string>();

    if (entry.contains("command")) {
      command = entry["command"].get<std::string>();
    } else if (entry.contains("arguments")) {
      const auto &args = entry["arguments"];
      if (args.is_array()) {
        for (const auto &arg : args)
          command += arg.get<std::string>() + " ";
      }
    } else {
      continue;
    }

    out.push_back({file, command, directory});
  }

  std::cerr << "[Info] Loaded " << out.size() << " compile commands.\n";
  return out;
}

} // namespace depdiscover