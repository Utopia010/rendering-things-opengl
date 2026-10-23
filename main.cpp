#include "TeapotShow.hpp"
#include <filesystem>
#include <unordered_map>

#include <GLFW/glfw3.h>
#include <glad/gl.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <iostream>
#include <vector>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <Camera.hpp>
#include <MyTools.hpp>
#include <OceanShow.hpp>
#include <OceanSurfaceSimulator.hpp>
#include <ShaderProgramManager.hpp>
#include <VAOManager.hpp>
#include <call_back_functions.hpp>
struct SurfaceMaterial {
  glm::vec3 Albedo;
  float Metallic;
  float Roughness;
  float Ao;
};
std::filesystem::path current_path = std::filesystem::current_path();

static double last_frame_time = 0.0;

int main() {

  // init glfw and set version of gl
  glfwSetErrorCallback(my::callback::error_callback);
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
      glfwCreateWindow(1600, 1200, "GLFW + GLAD + OpenGL", nullptr, nullptr);
  if (!window) {
    std::cerr << "Failed to create GLFW window\n";
    glfwTerminate();
    return -1;
  }
  glfwMakeContextCurrent(window);
  // load function pointer of OpenGl using glad
  if (!gladLoadGL(glfwGetProcAddress)) {
    std::cerr << "Failed to initialize GLAD\n";
    glfwDestroyWindow(window);
    glfwTerminate();
    return -1;
  }
  glEnable(GL_DEBUG_OUTPUT);
  glDebugMessageCallback(my::callback::gl_debug_callback,
                         nullptr); // set callback func
  glfwSetFramebufferSizeCallback(window,
                                 my::callback::resize_viewport_callback);
  glfwSetMouseButtonCallback(window, my::callback::my_mouse_button_callback);
  glfwSetCursorPosCallback(window, my::callback::my_cursor_position_callback);
  int winWidth, winHeight;
  glfwGetFramebufferSize(window, &winWidth, &winHeight);
  glViewport(0, 0, winWidth, winHeight);
  glfwSetKeyCallback(window, my::callback::key_callback);

  printf("glsl version:%s \n", glGetString(GL_SHADING_LANGUAGE_VERSION));
  int temp_texture_unit_number = 0;
  glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &temp_texture_unit_number);
  printf("max texture units:%d \n", temp_texture_unit_number);

  // Setup Dear ImGui context
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  io.ConfigFlags |=
      ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
  io.ConfigFlags |=
      ImGuiConfigFlags_NavEnableGamepad; // Enable Gamepad Controls
  ImGui_ImplGlfw_InitForOpenGL(window, false);
  ImGui_ImplOpenGL3_Init("#version 330");
  float scaleFactor = 2.0f;
  ImGui::GetStyle().ScaleAllSizes(scaleFactor);
  ImFont *font = io.Fonts->AddFontDefault();
  font->Scale = scaleFactor;

  /*buffers*/
  my::VBOAttributeConfig posConfig{0, 3, GL_FLOAT, sizeof(float) * 3, false};
  my::VBOAttributeConfig normConfig{1, 3, GL_FLOAT, sizeof(float) * 3, false};

  std::string teapot_model_path =
      current_path / std::string("Model/obj/utah_teapot.obj");
  std::filesystem::path vert_simple_file =
      current_path / std::filesystem::path("src/shaders/simple.vert");
  std::string frag_simple_file =
      current_path / std::filesystem::path("src/shaders/simple.frag");
  std::string vert_ocean_file = current_path / "src/shaders/ocean.vert";
  std::string frag_ocean_file = current_path / "src/shaders/ocean.frag";

  // 主循环
  glm::vec2 mouse_xy{0};
  bool first_init = true;
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);
  bool is_corsur_pressed = false;
  glClearColor(0.1f, 0.2f, 0.3f, 1.0f);

  my::Camera mainCamera{my::DefaultConfig};
  unsigned int selected_id = 0;
  std::vector<const char *> selected_list{"teapot", "ocean surface"};
  my::show::OceanSurface oceanSurface(vert_ocean_file, frag_ocean_file);
  my::show::TeapotShow teapotShow(vert_simple_file, frag_simple_file,
                                  teapot_model_path);
  while (!glfwWindowShouldClose(window)) {
    // deal with input
    glfwPollEvents();

    // clear
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // imgui new frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    // ImGui::ShowDemoWindow();
    mainCamera.update(window, io.WantCaptureMouse);
    if (selected_id == 1) {
      oceanSurface.update(mainCamera);
    } else if (selected_id == 0) {
      teapotShow.update(mainCamera);
    }
    {
      ImGui::Begin("Config"); // Create a window named "Data Display Window"

      // Display frame rate
      ImGui::Separator();
      ImGui::Text("Application average %.3f ms/frame (%.1FPS)",
                  1000.0f / io.Framerate, io.Framerate);

      ImGui::Text("pos: {%.1f, %.1f, %.1f}", mainCamera.config_.position.x,
                  mainCamera.config_.position.y, mainCamera.config_.position.z);
      ImGui::Text(
          "direction: {%.1f, %.1f, %.1f}", mainCamera.config_.direction.x,
          mainCamera.config_.direction.y, mainCamera.config_.direction.z);
      ImGui::Text("randian: yaw: %.1f pitch: %.1f", mainCamera.config_.yaw,
                  mainCamera.config_.pitch);

      double cursor_x, cursor_y;
      glfwGetCursorPos(window, &cursor_x, &cursor_y);
      ImGui::Text("cursor: x: %.1lf y: %.1lf", cursor_x, cursor_y);

      for (int i = 0; i < selected_list.size(); i++) {
        if (i != 0)
          ImGui::SameLine();
        if (ImGui::Button(selected_list[i])) {
          selected_id = i;
        }
      }
      if (selected_id == 0) {
        teapotShow.update(mainCamera);
      } else {
        oceanSurface.update(mainCamera);
      }
      ImGui::End();
    }

    // imgui rendering
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    // 把后端绘制结果显示到窗口
    glfwSwapBuffers(window);
    last_frame_time = glfwGetTime();
  }

  // 清理
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();
  glfwDestroyWindow(window);
  glfwTerminate();

  return 0;
}