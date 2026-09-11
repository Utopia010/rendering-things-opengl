#pragma once
#include <string>
#include <string_view>
#include <fstream>
#include <iostream>
#include <filesystem>

namespace MyTools {
inline bool ReadShaderSource(std::string_view path, std::string &source,
                             bool is_print = false) {
  if (!std::filesystem::exists(path)) {
    std::cout << "[warning]: file no existed => " << path.data() << '\n';
    return false;
  }
  source.clear();
  std::ifstream file{path.data()};
  std::string line{};
  int lineNumber = 0;
  if (is_print) {
    std::cout << "[FILE]: " << path << '\n';
  }
  while (std::getline(file, line)) {
    if (line.empty())
      continue;

    size_t result = line.find('/', 0);
    std::string_view temp = line;
    if (result != -1 && line[result + 1] == '/') {
      temp.remove_suffix(line.length() - result);
      if (temp.size() == 0)
        continue;
    }
    source += temp;
    source += '\n';
    if (is_print) {
      std::cout << lineNumber << ": " << temp << '\n';
      lineNumber++;
    }
  }
  return true;
}

}; // namespace MyTools