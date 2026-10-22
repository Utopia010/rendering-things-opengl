#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <string>
#include <unordered_map>
#include <iostream>
#include <filesystem>
#include <fstream>

namespace my {
enum class UniformType { Float, Vec3, Mat4 };
class ShaderProgram {
public:
  ShaderProgram(const std::string &vertexShaderFile,
                const std::string &fragmentShaderFile) {
    unsigned int VertexShaderHandler = glCreateShader(GL_VERTEX_SHADER);
    unsigned int FragmentShaderHandler = glCreateShader(GL_FRAGMENT_SHADER);
    const char *ShaderSourcePointer;
    int success;
    char buf[512];
    std::string vertexShaderSource, fragmentShaderSource;

    ReadShaderSource(vertexShaderFile, vertexShaderSource, true);
    ShaderSourcePointer = vertexShaderSource.c_str();
    glShaderSource(VertexShaderHandler, 1, &ShaderSourcePointer, NULL);
    glCompileShader(VertexShaderHandler);
    glGetShaderiv(VertexShaderHandler, GL_COMPILE_STATUS, &success);
    if (!success) {
      glGetShaderInfoLog(VertexShaderHandler, 512, nullptr, buf);
      std::printf("[ERROR]>[SHADER]>[VERTEX]>[COMPILE]: %s\n", buf);
    }
    ReadShaderSource(fragmentShaderFile, fragmentShaderSource, true);
    ShaderSourcePointer = fragmentShaderSource.c_str();
    glShaderSource(FragmentShaderHandler, 1, &ShaderSourcePointer, nullptr);
    glCompileShader(FragmentShaderHandler);
    glGetShaderiv(FragmentShaderHandler, GL_COMPILE_STATUS, &success);
    if (!success) {
      glGetShaderInfoLog(FragmentShaderHandler, 512, nullptr, buf);
      std::printf("[ERROR]>[SHADER]>[FRAGMENT]>[COMPILE]: %s\n", buf);
    }
    ShaderProgramHandler_ = glCreateProgram();
    glAttachShader(ShaderProgramHandler_, VertexShaderHandler);
    glAttachShader(ShaderProgramHandler_, FragmentShaderHandler);
    glLinkProgram(ShaderProgramHandler_);
    glGetProgramiv(ShaderProgramHandler_, GL_LINK_STATUS, &success);
    if (!success) {
      glGetProgramInfoLog(ShaderProgramHandler_, 512, nullptr, buf);
      std::printf("[ERROR]>[SHADER]>[PROGRAM]>[LINK]: %s\n", buf);
    }
    glDeleteShader(VertexShaderHandler);
    glDeleteShader(FragmentShaderHandler);
  }
  void bind() { glUseProgram(ShaderProgramHandler_); }
  void unbind() { glUseProgram(0); }
  void setUniform(const std::string name, UniformType type, float *ptr) {
    unsigned int location;
    if (!uniformLocations_.contains(name)) {
      const char *name_ptr = name.data();
      location = glGetUniformLocation(ShaderProgramHandler_, name_ptr);
      if (location == -1) {
        std::printf("[WARNING]: no such uniform: %s\n", name.c_str());
        return;
      }
      uniformLocations_[name] = location;
    } else
      location = uniformLocations_[name];
    switch (type) {
    case UniformType::Float:
      glProgramUniform1f(ShaderProgramHandler_, location, *ptr);
      break;
    case UniformType::Vec3:
      glProgramUniform3fv(ShaderProgramHandler_, location, 1, ptr);
      break;
    case UniformType::Mat4:
      glProgramUniformMatrix4fv(ShaderProgramHandler_, location, 1, false, ptr);
      break;
    default:
      std::printf("[WARINNING]: unkown enum type\n");
    }
  }

public:
  inline static bool ReadShaderSource(std::string_view path,
                                      std::string &source,
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

public:
  unsigned int ShaderProgramHandler_;
  std::unordered_map<std::string, unsigned int> uniformLocations_{};
};

}; // namespace my