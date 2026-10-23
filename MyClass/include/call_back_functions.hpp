#pragma once
#include <GLFW/glfw3.h>
#include <glad/gl.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <iostream>

namespace my {
namespace callback {
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
  ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods);
}
static void my_mouse_button_callback(GLFWwindow *window, int button, int action,
                                     int mods) {
  ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);
}
static void my_cursor_position_callback(GLFWwindow *window, double xpos,
                                        double ypos) {
  ImGui_ImplGlfw_CursorPosCallback(window, xpos, ypos);
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
} // namespace callback

} // namespace my