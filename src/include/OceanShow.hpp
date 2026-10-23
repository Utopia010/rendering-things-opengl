#pragma once

#include "OceanSurfaceSimulator.hpp"
#include <Camera.hpp>
#include <GLFW/glfw3.h>
#include <ShaderProgramManager.hpp>
#include <VAOManager.hpp>
#include <glad/gl.h>
#include <glm/gtc/type_ptr.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
namespace my {
namespace show {
class OceanSurface {
public:
  OceanSurface(const std::string &vert_file, const std::string &frag_file,
               OceanConfig config = OceanConfig{})
      : config_(config), ocean_(config), program_(vert_file, frag_file) {
    init();
  }

  void update(my::Camera &camera) {
    ocean_.update(glfwGetTime());
    auto ocean_heights = ocean_.heights();
    for (int i = 0; i < ocean_heights.size(); i++)
      for (int j = 0; j < ocean_heights[i].size(); j++) {
        ocean_pos_[i * ocean_heights.size() + j].y = ocean_heights[i][j] * 1;
      }

    program_.setUniform("mvp_model", my::UniformType::Mat4,
                        glm::value_ptr(ocean_mvp_model_));
    program_.setUniform("mvp_view", my::UniformType::Mat4,
                        glm::value_ptr(camera.mvp_view_));
    program_.setUniform("mvp_proj", my::UniformType::Mat4,
                        glm::value_ptr(camera.mvp_project_));

    program_.bind();
    vao_.bind();
    vao_.updateVBOData((void *)&ocean_pos_[0],
                       sizeof(ocean_pos_[0]) * ocean_pos_.size());
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glDrawElements(GL_TRIANGLES, vao_.getEBOCount(), GL_UNSIGNED_INT,
                   (void *)0);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  }

private:
  void init() {
    auto ocean_heights = ocean_.heights();
    for (int i = 0; i < ocean_heights.size() - 1; i++) {
      for (int j = 0; j < ocean_heights[i].size() - 1; j++) {
        int temp_index = i * config_.samplesX + j;
        ocean_index_.push_back(temp_index + 1);
        ocean_index_.push_back(temp_index);
        ocean_index_.push_back(temp_index + config_.samplesX);
        ocean_index_.push_back(temp_index + 1);
        ocean_index_.push_back(temp_index + config_.samplesX);
        ocean_index_.push_back(temp_index + config_.samplesX + 1);
      }
    }

    for (int i = 0; i < ocean_heights.size(); i++)
      for (int j = 0; j < ocean_heights[i].size(); j++) {
        ocean_pos_.emplace_back(i * 0.1, ocean_heights[i][j] * 0.05, j * 0.1);
      }

    vao_.addAttributeLayout(posConfig);
    vao_.addEBOData(ocean_index_);
    vao_.addVBOData(glm::value_ptr(ocean_pos_[0]),
                    sizeof(ocean_pos_[0]) * ocean_pos_.size(), true);
  }

public:
  OceanConfig config_;
  OceanSurfaceSimulator ocean_;
  std::vector<unsigned int> ocean_index_{};
  std::vector<glm::vec3> ocean_pos_{};
  my::VAOManager vao_{};
  my::VBOAttributeConfig posConfig{0, 3, GL_FLOAT, sizeof(float) * 3, false};
  my::ShaderProgram program_;
  glm::mat4 ocean_mvp_model_{1.0f};
};
}; // namespace show
}; // namespace my