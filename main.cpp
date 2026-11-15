#include <cstdio>
#include <cstdlib>
#include <exception>
#include <filesystem>
#include <glm/geometric.hpp>
#include <unordered_map>

#include <glad/gl.h>

#include <GLFW/glfw3.h>

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
#include <TeapotShow.hpp>
#include <VAOManager.hpp>
#include <call_back_functions.hpp>
#include <stb_image.h>

struct SurfaceMaterial {
  glm::vec3 Albedo;
  float Metallic;
  float Roughness;
  float Ao;
};
std::filesystem::path current_path = std::filesystem::current_path();
void img_flipHorizontal(unsigned char *data, int width, int height,
                        int channels) {
  for (int y = 0; y < height; ++y) {
    for (int x = 0; x < width / 2; ++x) {
      int left = (y * width + x) * channels;
      int right = (y * width + width - 1 - x) * channels;

      for (int c = 0; c < channels; ++c) {
        std::swap(data[left + c], data[right + c]);
      }
    }
  }
}
unsigned char *img_rotate90CW(const unsigned char *src, int width, int height,
                              int channels) {
  if (!src || width <= 0 || height <= 0 || channels <= 0)
    return nullptr;

  unsigned char *dst = new unsigned char[width * height * channels];

  for (int y = 0; y < height; ++y) {
    for (int x = 0; x < width; ++x) {
      int srcIndex = (y * width + x) * channels;

      // 顺时针 90°
      int newX = height - 1 - y;
      int newY = x;

      int dstIndex = (newY * height + newX) * channels;

      for (int c = 0; c < channels; ++c) {
        dst[dstIndex + c] = src[srcIndex + c];
      }
    }
  }

  return dst;
}
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
  auto temp = glm::cross(my::UP, my::FRONT);
  std::printf("(%f, %f, %f)\n", temp.x, temp.y, temp.z);
  // cube
  float scale_ = 1;
  std::vector<glm::vec3> cubePositions = {
      glm::vec3(-0.5f, -0.5f, -0.5f), // 0
      glm::vec3(0.5f, -0.5f, -0.5f),  // 1
      glm::vec3(0.5f, 0.5f, -0.5f),   // 2
      glm::vec3(-0.5f, 0.5f, -0.5f),  // 3
      glm::vec3(-0.5f, -0.5f, 0.5f),  // 4
      glm::vec3(0.5f, -0.5f, 0.5f),   // 5
      glm::vec3(0.5f, 0.5f, 0.5f),    // 6
      glm::vec3(-0.5f, 0.5f, 0.5f)    // 7
  };
  for (auto &e : cubePositions) {
    e *= scale_;
  }

  std::vector<unsigned int> cubeIndices = {0, 1, 2, 0, 2, 3,  // 后面
                                           4, 5, 6, 4, 6, 7,  // 前面
                                           0, 4, 7, 0, 7, 3,  // 左面
                                           1, 5, 6, 1, 6, 2,  // 右面
                                           3, 7, 6, 3, 6, 2,  // 顶面
                                           0, 1, 5, 0, 5, 4}; // 底面
  my::VAOManager vao_cube{};
  vao_cube.addEBOData(cubeIndices);
  vao_cube.addAttributeLayout(posConfig);
  vao_cube.addVBOData((void *)&cubePositions[0],
                      cubePositions.size() * sizeof(cubePositions[0]));
  // sky box
  std::unordered_map<GLenum, std::filesystem::path> skybox_files;
  skybox_files[GL_TEXTURE_CUBE_MAP_NEGATIVE_Z] =
      current_path / "Model/skybox/sky_01_cubemap_2k/px.png";
  skybox_files[GL_TEXTURE_CUBE_MAP_POSITIVE_Z] =
      current_path / "Model/skybox/sky_01_cubemap_2k/nx.png";
  skybox_files[GL_TEXTURE_CUBE_MAP_POSITIVE_Y] =
      current_path / "Model/skybox/sky_01_cubemap_2k/py.png";
  skybox_files[GL_TEXTURE_CUBE_MAP_NEGATIVE_Y] =
      current_path / "Model/skybox/sky_01_cubemap_2k/ny.png";
  skybox_files[GL_TEXTURE_CUBE_MAP_NEGATIVE_X] =
      current_path / "Model/skybox/sky_01_cubemap_2k/nz.png";
  skybox_files[GL_TEXTURE_CUBE_MAP_POSITIVE_X] =
      current_path / "Model/skybox/sky_01_cubemap_2k/pz.png";
  unsigned int skyboxHandler;
  glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &skyboxHandler);
  glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxHandler);

  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
  stbi_set_flip_vertically_on_load(false);
  unsigned char *img_data;
  int width, height, channel;
  for (const auto &[key, value] : skybox_files) {

    if (key == GL_TEXTURE_CUBE_MAP_POSITIVE_Y) {
      stbi_set_flip_vertically_on_load(true);
      img_data = stbi_load(value.c_str(), &width, &height, &channel, 3);
      stbi_set_flip_vertically_on_load(false);
      img_flipHorizontal(img_data, width, height, 3);
      auto temp = img_rotate90CW(img_data, width, height, 3);
      stbi_image_free(img_data);
      glTexImage2D(key, 0, GL_RGB, height, width, 0, GL_RGB, GL_UNSIGNED_BYTE,
                   temp);
      delete temp;

    } else {
      img_data = stbi_load(value.c_str(), &width, &height, &channel, 3);
      glTexImage2D(key, 0, GL_RGB, height, width, 0, GL_RGB, GL_UNSIGNED_BYTE,
                   img_data);
      stbi_image_free(img_data);
    }
  }
  glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
  glBindTextureUnit(0, skyboxHandler);

  // hdr skybox
  std::string hdr_path = current_path / "Model/skybox/809-hdri-skies-com.hdr";
  float *img_dataf;
  stbi_set_flip_vertically_on_load(true);
  img_dataf = stbi_loadf(hdr_path.c_str(), &width, &height, &channel, 3);
  if (!img_dataf) {
    std::printf("[ERROR] > [HDR] > [OPEN] : path %s\n", hdr_path.c_str());
    return -1;
  }
  unsigned int hdrSkyboxHandler;
  glGenTextures(1, &hdrSkyboxHandler);
  glBindTexture(GL_TEXTURE_2D, hdrSkyboxHandler);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT,
               img_dataf);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glGenerateMipmap(hdrSkyboxHandler);
  glBindTextureUnit(1, hdrSkyboxHandler);
  stbi_image_free(img_dataf);

  std::string hdr_shader_vert = current_path / "src/shaders/hdr-skybox.vert";
  std::string hdr_shader_frag = current_path / "src/shaders/hdr-skybox.frag";
  my::ShaderProgram hdr_program(hdr_shader_vert, hdr_shader_frag, true);
  int hdr_skybox_index = 1;
  hdr_program.setUniform("skybox", my::UniformType::Int,
                         (void *)&hdr_skybox_index);
  float hdr_dividand = 1.0;
  hdr_program.setUniform("dividand", my::UniformType::Float,
                         (void *)&hdr_dividand);

  std::string skybox_vert_file = current_path / "src/shaders/cube.vert";
  std::string skybox_frag_file = current_path / "src/shaders/cube.frag";

  my::ShaderProgram skybox_program(skybox_vert_file, skybox_frag_file, true);
  int skybox_sample_index = 0;
  skybox_program.setUniform("skybox", my::UniformType::Int,
                            (void *)&skybox_sample_index);
  glm::mat4 skybox_view;

  // 主循环
  glm::vec2 mouse_xy{0};
  bool first_init = true;
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LEQUAL);
  bool is_corsur_pressed = false;
  glClearColor(0.1f, 0.2f, 0.3f, 1.0f);

  my::Camera mainCamera{my::DefaultConfig};
  unsigned int selected_id = 0;
  std::vector<const char *> selected_list{"teapot", "ocean surface", "skybox",
                                          "hdr-skybox"};
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
      } else if (selected_id == 1) {
        oceanSurface.update(mainCamera);
      } else if (selected_id == 2) {
        skybox_view = glm::mat4(glm::mat3(mainCamera.mvp_view_));
        vao_cube.bind();
        skybox_program.bind();
        skybox_program.setUniform("mvp_view", my::UniformType::Mat4,
                                  glm::value_ptr(skybox_view));
        skybox_program.setUniform("mvp_proj", my::UniformType::Mat4,
                                  glm::value_ptr(mainCamera.mvp_project_));
        glDrawElements(GL_TRIANGLES, vao_cube.getEBOCount(), GL_UNSIGNED_INT,
                       (void *)0);
      } else {
        vao_cube.bind();
        skybox_view = glm::mat4(glm::mat3(mainCamera.mvp_view_));
        hdr_program.bind();
        hdr_program.setUniform("mvp_view", my::UniformType::Mat4,
                               glm::value_ptr(mainCamera.mvp_view_));
        hdr_program.setUniform("mvp_proj", my::UniformType::Mat4,
                               glm::value_ptr(mainCamera.mvp_project_));
        glDrawElements(GL_TRIANGLES, vao_cube.getEBOCount(), GL_UNSIGNED_INT,
                       (void *)0);
        hdr_program.setUniform("dividand", my::UniformType::Float,
                               (void *)&hdr_dividand);
        ImGui::DragFloat("dividand", &hdr_dividand, 0.001, 0.5, 1.f);
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