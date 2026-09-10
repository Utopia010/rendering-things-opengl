#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
static void error_callback(int error, const char *description) {
  fprintf(stderr, "[GLFW Error] %d: %s\n", error, description);
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

  // 7. 主循环
  while (!glfwWindowShouldClose(window)) {
    // 设置清屏颜色
    glClearColor(0.1f, 0.2f, 0.3f, 1.0f);

    // 清除颜色缓冲
    glClear(GL_COLOR_BUFFER_BIT);

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