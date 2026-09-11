#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include <vector>
#include <iostream>

#include <MyTools.hpp>
static void error_callback(int error, const char *description) {
  fprintf(stderr, "[GLFW Error] %d: %s\n", error, description);
}
static void resize_viewport_callback(GLFWwindow *window, int width,
                                     int height) {

  glViewport(0, 0, width, height);
}
static void key_callback(GLFWwindow *window, int key, int scancode, int action,
                         int mods) {
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, true);
  }
}
static void APIENTRY gl_debug_callback(GLenum source, GLenum type, GLuint id,
                                       GLenum severity, GLsizei length,
                                       const GLchar *message,
                                       const void *userParam) {
  (void)length;
  (void)userParam;

  // 将枚举值转换为可读字符串
  const char *source_str;
  switch (source) {
  case GL_DEBUG_SOURCE_API:
    source_str = "API";
    break;
  case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
    source_str = "Window System";
    break;
  case GL_DEBUG_SOURCE_SHADER_COMPILER:
    source_str = "Shader Compiler";
    break;
  case GL_DEBUG_SOURCE_THIRD_PARTY:
    source_str = "Third Party";
    break;
  case GL_DEBUG_SOURCE_APPLICATION:
    source_str = "Application";
    break;
  case GL_DEBUG_SOURCE_OTHER:
    source_str = "Other";
    break;
  default:
    source_str = "Unknown";
    break;
  }

  const char *type_str;
  switch (type) {
  case GL_DEBUG_TYPE_ERROR:
    type_str = "Error";
    break;
  case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
    type_str = "Deprecated";
    break;
  case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
    type_str = "Undefined";
    break;
  case GL_DEBUG_TYPE_PORTABILITY:
    type_str = "Portability";
    break;
  case GL_DEBUG_TYPE_PERFORMANCE:
    type_str = "Performance";
    break;
  case GL_DEBUG_TYPE_MARKER:
    type_str = "Marker";
    break;
  case GL_DEBUG_TYPE_PUSH_GROUP:
    type_str = "Push Group";
    break;
  case GL_DEBUG_TYPE_POP_GROUP:
    type_str = "Pop Group";
    break;
  case GL_DEBUG_TYPE_OTHER:
    type_str = "Other";
    break;
  default:
    type_str = "Unknown";
    break;
  }

  // 根据严重性选择输出流和前缀
  const char *severity_str;
  FILE *out = stderr;
  switch (severity) {
  case GL_DEBUG_SEVERITY_HIGH:
    severity_str = "HIGH";
    break;
  case GL_DEBUG_SEVERITY_MEDIUM:
    severity_str = "MEDIUM";
    break;
  case GL_DEBUG_SEVERITY_LOW:
    severity_str = "LOW";
    break;
  case GL_DEBUG_SEVERITY_NOTIFICATION:
    severity_str = "NOTIFICATION";
    out = stdout;
    break;
  default:
    severity_str = "UNKNOWN";
    break;
  }

  fprintf(out, "[OpenGL][%s][%s][%s] (id=%u): %s\n", severity_str, source_str,
          type_str, id, message);
  fflush(out);
}

int main() {

  // init glfw and set version of gl
  glfwSetErrorCallback(error_callback);
  if (!glfwInit()) {
    std::cerr << "Failed to initialize GLFW\n";
    return -1;
  }
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE); // set on debug of gl
  // set context of window
  GLFWwindow *window =
      glfwCreateWindow(800, 600, "GLFW + GLAD + OpenGL", nullptr, nullptr);
  if (!window) {
    std::cerr << "Failed to create GLFW window\n";
    glfwTerminate();
    return -1;
  }
  glfwMakeContextCurrent(window);
  // load function pointer of OpenGl using glad
  if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
    std::cerr << "Failed to initialize GLAD\n";
    glfwDestroyWindow(window);
    glfwTerminate();
    return -1;
  }
  glEnable(GL_DEBUG_OUTPUT);
  glDebugMessageCallback(gl_debug_callback, nullptr); // set callback func
  glfwSetFramebufferSizeCallback(window, resize_viewport_callback);
  int winWidth, winHeight;
  glfwGetFramebufferSize(window, &winWidth, &winHeight);
  glViewport(0, 0, winWidth, winHeight);
  glfwSetKeyCallback(window, key_callback);

  // data
  std::vector<glm::vec2> pos{{-0.5f, -0.5f}, {0.5f, -0.5f}, {0.0f, 0.5f}};
  std::vector<unsigned int> indices{0, 1, 2};

  /*buffers*/
  unsigned int VAO, VBO, EBO;
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);
  glGenBuffers(1, &EBO);
  // upload data
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, pos.size() * sizeof(glm::vec2), pos.data(),
               GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int),
               indices.data(), GL_STATIC_DRAW);
  // bind
  glBindVertexArray(VAO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void *)0);
  glEnableVertexAttribArray(0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  glBindVertexArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  /*shaders*/
  unsigned int vertexShaderHandler, fragmentShaderHandler;
  vertexShaderHandler = glCreateShader(GL_VERTEX_SHADER);
  fragmentShaderHandler = glCreateShader(GL_FRAGMENT_SHADER);
  std::string vertexShaderSource, fragmentShaderSource;
  MyTools::ReadShaderSource(
      "/home/utopia010/project/opengl-particle/src/shaders/simple.vert",
      vertexShaderSource, true);
  MyTools::ReadShaderSource(
      "/home/utopia010/project/opengl-particle/src/shaders/simple.frag",
      fragmentShaderSource, true);
  const char *tempVertexSource = vertexShaderSource.data();
  const char *tempFragSource = fragmentShaderSource.data();
  glShaderSource(vertexShaderHandler, 1, &tempVertexSource, nullptr);
  glShaderSource(fragmentShaderHandler, 1, &tempFragSource, nullptr);
  glCompileShader(vertexShaderHandler);
  int success;
  glGetShaderiv(vertexShaderHandler, GL_COMPILE_STATUS, &success);
  if (!success) {
    char buf[512] = {0};
    glGetShaderInfoLog(vertexShaderHandler, 512, nullptr, buf);
    std::printf("[ERROR]>[SHADER]>[VERTEX]>[COMPILE]: %s\n", buf);
  }
  glCompileShader(fragmentShaderHandler);
  glGetShaderiv(fragmentShaderHandler, GL_COMPILE_STATUS, &success);
  if (!success) {
    char buf[512] = {0};
    glGetShaderInfoLog(fragmentShaderHandler, 512, nullptr, buf);
    std::printf("[ERROR]>[SHADER]>[FRAGMENT]>[COMPILE]: %s\n", buf);
  }
  unsigned int shaderProgramHandler = glCreateProgram();
  glAttachShader(shaderProgramHandler, vertexShaderHandler);
  glAttachShader(shaderProgramHandler, fragmentShaderHandler);
  glLinkProgram(shaderProgramHandler);
  glGetProgramiv(shaderProgramHandler, GL_LINK_STATUS, &success);
  if (!success) {
    char buf[512] = {0};
    glGetProgramInfoLog(shaderProgramHandler, 512, nullptr, buf);
    std::printf("[ERROR]>[SHADER]>[PROGRAM]>[LINK]: %s\n", buf);
  }
  glDeleteShader(vertexShaderHandler);
  glDeleteShader(fragmentShaderHandler);
  // 7. 主循环
  while (!glfwWindowShouldClose(window)) {
    // 设置清屏颜色
    glClearColor(0.1f, 0.2f, 0.3f, 1.0f);

    // 清除颜色缓冲
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(shaderProgramHandler);
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, (void *)0);

    // 把后端绘制结果显示到窗口
    glfwSwapBuffers(window);

    // 处理键盘、鼠标、窗口事件
    glfwPollEvents();
  }

  // 8. 清理
  glfwDestroyWindow(window);
  glfwTerminate();

  return 0;
}