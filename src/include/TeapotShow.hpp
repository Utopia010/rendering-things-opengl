#pragma once
#include "Camera.hpp"
#include <MyTools.hpp>
#include <ShaderProgramManager.hpp>
#include <VAOManager.hpp>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <filesystem>
#include <glm/ext/vector_float3.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/mat4x4.hpp>
#include <string>
namespace my {
namespace show {
struct SurfaceMaterial {
  glm::vec3 Albedo;
  float Metallic;
  float Roughness;
  float Ao;
};
class TeapotShow {
public:
  TeapotShow(const std::string &vert_file, const std::string &frag_file,
             const std::string &teapot_path)
      : program_(vert_file, frag_file) {
    init(teapot_path);
  }

private:
  void init(const std::string &teapot_path) {
    mvp_model_ = glm::rotate(mvp_model_, glm::radians(-90.0f),
                             glm::vec3(1.0f, 0.0f, 0.0f));
    my::VBOAttributeConfig posConfig{0, 3, GL_FLOAT, sizeof(float) * 3, false};
    my::VBOAttributeConfig normConfig{1, 3, GL_FLOAT, sizeof(float) * 3, false};
    vao_.addAttributeLayout(posConfig);
    vao_.addAttributeLayout(normConfig);

    std::vector<unsigned int> teapot_indices;
    std::vector<MyTools::VertexPN> teapot;

    Assimp::Importer model_loader;
    const aiScene *scene = model_loader.ReadFile(
        teapot_path.data(), aiProcess_Triangulate | aiProcess_FlipUVs);
    if (scene == nullptr || scene->mRootNode == nullptr ||
        scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) {
      std::printf("[ERROR]>[MODEL]>[ASSIMP]: model file: %s\n",
                  teapot_path.data());
      return;
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
    vao_.addEBOData(teapot_indices);
    vao_.addVBOData((void *)&teapot[0], sizeof(teapot[0]) * teapot.size(),
                    false);
  }

public:
  void update(my::Camera &camera) {
    program_.setUniform("mvp_model", my::UniformType::Mat4,
                        glm::value_ptr(mvp_model_));
    program_.setUniform("mvp_view", my::UniformType::Mat4,
                        glm::value_ptr(camera.mvp_view_));
    program_.setUniform("mvp_proj", my::UniformType::Mat4,
                        glm::value_ptr(camera.mvp_project_));
    program_.setUniform("lightPos", my::UniformType::Vec3,
                        glm::value_ptr(lightPos_));
    program_.setUniform("lightColor", my::UniformType::Vec3,
                        glm::value_ptr(lightColor_));
    program_.setUniform("cameraPos", my::UniformType::Vec3,
                        glm::value_ptr(camera.config_.position));
    program_.setUniform("teapotAlbedo", my::UniformType::Vec3,
                        glm::value_ptr(teapotMaterial_.Albedo));
    program_.setUniform("teapotAo", my::UniformType::Float,
                        &teapotMaterial_.Ao);
    program_.setUniform("teapotRoughness", my::UniformType::Float,
                        &teapotMaterial_.Roughness);
    program_.setUniform("teapotMetallic", my::UniformType::Float,
                        &teapotMaterial_.Metallic);
    program_.bind();
    vao_.bind();
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDrawElements(GL_TRIANGLES, vao_.getEBOCount(), GL_UNSIGNED_INT,
                   (void *)0);
  }

public:
  my::ShaderProgram program_;
  glm::mat4 mvp_model_{1.0f};
  my::VAOManager vao_{};
  glm::vec3 lightPos_{0, 3, 1};
  glm::vec3 lightColor_{0.7, 0.8, 0.75};
  SurfaceMaterial teapotMaterial_{{0.3f, 0.8f, 0.4f}, 0.1f, 0.1f, 0.1f};
};
}; // namespace show
}; // namespace my