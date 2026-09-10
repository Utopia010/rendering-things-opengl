#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

bool init() {
  if (!glfwInit()) {
    std::cerr << "Failed to initialize GLFW\n";
    return false;
  }
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  // set context of window
  GLFWwindow *window =
      glfwCreateWindow(800, 600, "GLFW + GLAD + OpenGL", nullptr, nullptr);
  if (!window) {
    std::cerr << "Failed to create GLFW window\n";
    glfwTerminate();
    return false;
  }
  glfwMakeContextCurrent(window);
  // load function pointer of OpenGl using glad
  if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
    std::cerr << "Failed to initialize GLAD\n";
    glfwDestroyWindow(window);
    glfwTerminate();
    return false;
  }
  return true;
}

int main() {
  if (!init()) {
    std::printf("failed to init\n");
  }

  std::cout << "OpenGL Vendor  : " << glGetString(GL_VENDOR) << '\n';

  std::cout << "OpenGL Renderer: " << glGetString(GL_RENDERER) << '\n';

  std::cout << "OpenGL Version : " << glGetString(GL_VERSION) << '\n';

  std::cout << "GLSL Version   : " << glGetString(GL_SHADING_LANGUAGE_VERSION)
            << '\n';
}