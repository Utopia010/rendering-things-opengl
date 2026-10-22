#include <filesystem>
#include <unordered_map>
#define GLFW_INCLUDE_NONE
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
#include <OceanSurfaceSimulator.hpp>
#include <ShaderProgramManager.hpp>
#include <VAOManager.hpp>
struct SurfaceMaterial {
  glm::vec3 Albedo;
  float Metallic;
  float Roughness;
  float Ao;
};
std::filesystem::path current_path = std::filesystem::current_path();
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
void my_mouse_button_callback(GLFWwindow *window, int button, int action,
                              int mods) {
  ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);
}
void my_cursor_position_callback(GLFWwindow *window, double xpos, double ypos) {
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

static double last_frame_time = 0.0;

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
  glDebugMessageCallback(gl_debug_callback, nullptr); // set callback func
  glfwSetFramebufferSizeCallback(window, resize_viewport_callback);
  glfwSetMouseButtonCallback(window, my_mouse_button_callback);
  glfwSetCursorPosCallback(window, my_cursor_position_callback);
  int winWidth, winHeight;
  glfwGetFramebufferSize(window, &winWidth, &winHeight);
  glViewport(0, 0, winWidth, winHeight);
  glfwSetKeyCallback(window, key_callback);

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

  /*load data*/
  std::vector<unsigned int> teapot_indices;
  std::vector<MyTools::VertexPN> teapot;

  Assimp::Importer model_loader;
  SurfaceMaterial teapotMaterial{{0.3f, 0.8f, 0.4f}, 0.1f, 0.1f, 0.1f};
  std::string tea_pot = current_path / std::string("Model/obj/utah_teapot.obj");
  const aiScene *scene = model_loader.ReadFile(
      tea_pot.data(), aiProcess_Triangulate | aiProcess_FlipUVs);
  if (scene == nullptr || scene->mRootNode == nullptr ||
      scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) {
    std::printf("[ERROR]>[MODEL]>[ASSIMP]: model file: %s\n", tea_pot.data());
    return -1;
  }

  for (int m = 0; m < scene->mNumMeshes; m++) {
    aiMesh *mesh = scene->mMeshes[m];
    unsigned int indices_base = static_cast<unsigned int>(teapot.size());
    // pos
    for (int i = 0; i < mesh->mNumVertices; i++) {
      aiVector3D &t_pos = mesh->mVertices[i];
      aiVector3D &t_norm = mesh->mNormals[i];
      teapot.push_back({{t_pos.x / 10, t_pos.y / 10, t_pos.z / 10},
                        {t_norm.x, t_norm.y, t_norm.z}});
    }
    // indices
    for (int i = 0; i < mesh->mNumFaces; i++) {
      aiFace &temp = mesh->mFaces[i];
      for (int j = 0; j < temp.mNumIndices; j++) {
        teapot_indices.push_back(indices_base + temp.mIndices[j]);
      }
    }
  }

  OceanConfig ocean_config{};
  ocean_config.samplesX = 128;
  ocean_config.samplesZ = 128;
  OceanSurfaceSimulator ocean{ocean_config};
  auto ocean_heights = ocean.heights();
  std::vector<unsigned int> ocean_index{};
  std::vector<glm::vec3> ocean_pos{};
  for (int i = 0; i < ocean_heights.size() - 1; i++) {
    for (int j = 0; j < ocean_heights[i].size() - 1; j++) {
      int temp_index = i * ocean_config.samplesX + j;
      ocean_index.push_back(temp_index + 1);
      ocean_index.push_back(temp_index);
      ocean_index.push_back(temp_index + ocean_config.samplesX);
      ocean_index.push_back(temp_index + 1);
      ocean_index.push_back(temp_index + ocean_config.samplesX);
      ocean_index.push_back(temp_index + ocean_config.samplesX + 1);
    }
  }
  for (int i = 0; i < ocean_heights.size(); i++)
    for (int j = 0; j < ocean_heights[i].size(); j++) {
      ocean_pos.emplace_back(i * 0.1, ocean_heights[i][j] * 0.05, j * 0.1);
    }

  /*buffers*/
  my::VBOAttributeConfig posConfig{0, 3, GL_FLOAT, sizeof(float) * 3, false};
  my::VBOAttributeConfig normConfig{1, 3, GL_FLOAT, sizeof(float) * 3, false};

  my::VAOManager vao_ocean{};
  vao_ocean.addAttributeLayout(posConfig);
  vao_ocean.addEBOData(ocean_index);
  vao_ocean.addVBOData(glm::value_ptr(ocean_pos[0]),
                       sizeof(ocean_pos[0]) * ocean_pos.size(), true);
  my::VAOManager vao_teapot{};
  vao_teapot.addAttributeLayout(posConfig);
  vao_teapot.addAttributeLayout(normConfig);
  vao_teapot.addEBOData(teapot_indices);
  vao_teapot.addVBOData((void *)&teapot[0], sizeof(teapot[0]) * teapot.size(),
                        false);

  std::filesystem::path vert_simple_file =
      current_path / std::filesystem::path("src/shaders/simple.vert");
  std::string frag_simple_file =
      current_path / std::filesystem::path("src/shaders/simple.frag");
  std::string vert_ocean_file = current_path / "src/shaders/ocean.vert";
  std::string frag_ocean_file = current_path / "src/shaders/ocean.frag";
  my::ShaderProgram oceanProgram{vert_ocean_file, frag_ocean_file};
  my::ShaderProgram teapotProgram{vert_simple_file, frag_simple_file};

  /*MVP*/
  glm::mat4 mvp_model_ocean{1.0f};
  glm::mat4 mvp_model_teapot{1.0f};
  mvp_model_teapot = glm::rotate(mvp_model_teapot, glm::radians(-90.0f),
                                 glm::vec3(1.0f, 0.0f, 0.0f));
  glm::vec3 lightPos{0, 3, 1};
  glm::vec3 lightColor{0.7, 0.8, 0.75};

  // 主循环
  glm::vec2 mouse_xy{0};
  bool first_init = true;
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);
  glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
  bool is_corsur_pressed = false;
  glClearColor(0.1f, 0.2f, 0.3f, 1.0f);

  my::Camera mainCamera{my::DefaultConfig};
  unsigned int selected_id = 0;
  std::vector<const char *> selected_list{"teapot", "ocean surface"};
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
      ocean.update(glfwGetTime());
      ocean_heights = ocean.heights();
      for (int i = 0; i < ocean_heights.size(); i++)
        for (int j = 0; j < ocean_heights[i].size(); j++) {
          ocean_pos[i * ocean_heights.size() + j].y = ocean_heights[i][j] * 1;
        }

      oceanProgram.setUniform("mvp_model", my::UniformType::Mat4,
                              glm::value_ptr(mvp_model_ocean));
      oceanProgram.setUniform("mvp_view", my::UniformType::Mat4,
                              glm::value_ptr(mainCamera.mvp_view_));
      oceanProgram.setUniform("mvp_proj", my::UniformType::Mat4,
                              glm::value_ptr(mainCamera.mvp_project_));
      oceanProgram.bind();
      vao_ocean.bind();
      vao_ocean.updateVBOData((void *)&ocean_pos[0],
                              sizeof(ocean_pos[0]) * ocean_pos.size());
      glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
      glDrawElements(GL_TRIANGLES, vao_ocean.getEBOCount(), GL_UNSIGNED_INT,
                     (void *)0);
    } else if (selected_id == 0) {
      teapotProgram.setUniform("mvp_model", my::UniformType::Mat4,
                               glm::value_ptr(mvp_model_teapot));
      teapotProgram.setUniform("mvp_view", my::UniformType::Mat4,
                               glm::value_ptr(mainCamera.mvp_view_));
      teapotProgram.setUniform("mvp_proj", my::UniformType::Mat4,
                               glm::value_ptr(mainCamera.mvp_project_));
      teapotProgram.setUniform("lightPos", my::UniformType::Vec3,
                               glm::value_ptr(lightPos));
      teapotProgram.setUniform("lightColor", my::UniformType::Vec3,
                               glm::value_ptr(lightColor));
      teapotProgram.setUniform("cameraPos", my::UniformType::Vec3,
                               glm::value_ptr(mainCamera.config_.position));
      teapotProgram.setUniform("teapotAlbedo", my::UniformType::Vec3,
                               glm::value_ptr(teapotMaterial.Albedo));
      teapotProgram.setUniform("teapotAo", my::UniformType::Float,
                               &teapotMaterial.Ao);
      teapotProgram.setUniform("teapotRoughness", my::UniformType::Float,
                               &teapotMaterial.Roughness);
      teapotProgram.setUniform("teapotMetallic", my::UniformType::Float,
                               &teapotMaterial.Metallic);
      teapotProgram.bind();
      vao_teapot.bind();
      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
      glDrawElements(GL_TRIANGLES, vao_teapot.getEBOCount(), GL_UNSIGNED_INT,
                     (void *)0);
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
        ImGui::BeginGroup();
        ImGui::Text("light pos: x: %.1f y: %.1f z: %.1f", lightPos.x,
                    lightPos.y, lightPos.z);
        ImGui::ColorEdit3("teapot color",
                          glm::value_ptr(teapotMaterial.Albedo));
        ImGui::SliderFloat("roughness", &teapotMaterial.Roughness, 0, 1);
        ImGui::SliderFloat("metallic", &teapotMaterial.Metallic, 0, 1);
        ImGui::EndGroup();
        ImVec2 group_min = ImGui::GetItemRectMin();
        ImVec2 group_max = ImGui::GetItemRectMax();
        ImDrawList *draw_list = ImGui::GetWindowDrawList();
        float padding = 4.0f;
        draw_list->AddRectFilled(
            ImVec2(group_min.x - padding, group_min.y - padding),
            ImVec2(group_max.x + padding, group_max.y + padding),
            IM_COL32(20, 55, 105, 120));
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