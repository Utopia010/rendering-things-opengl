#pragma once
#include <string>
#include <string_view>
#include <fstream>
#include <iostream>
#include <filesystem>
#include <glm/vec3.hpp>

namespace MyTools {
struct VertexP {
  glm::vec3 pos;
};
struct VertexPN {
  glm::vec3 pos;
  glm::vec3 norm;
};


}; // namespace MyTools