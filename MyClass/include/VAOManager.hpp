#pragma once

#include <cstdint>
#include <glad/gl.h>
#include <iostream>
#include <vector>

namespace my {
struct VBOAttributeConfig {
  unsigned int index;
  int count;
  GLenum type;
  int size;
  bool Normlised = false;
};
class VAOManager {
public:
  VAOManager() {
    glGenVertexArrays(1, &VAOHandler_);
    glGenBuffers(1, &VBOHandler_);
    glGenBuffers(1, &EBOHandler_);
  }
  void bind() const { glBindVertexArray(VAOHandler_); }
  void unbind() const { glBindVertexArray(0); }
  void addVBOData(void *data, GLsizeiptr size, bool is_changed = false) {
    is_changed_ = is_changed;
    bind();
    glBindBuffer(GL_ARRAY_BUFFER, VBOHandler_);
    glBufferData(GL_ARRAY_BUFFER, size, data,
                 is_changed_ ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);

    int offset = 0;
    for (int i = 0; i < layouts_.size(); i++) {
      glEnableVertexAttribArray(i);
      glVertexAttribPointer(layouts_[i].index, layouts_[i].count,
                            layouts_[i].type, layouts_[i].Normlised, step_,
                            (void *)(uintptr_t)offset);
      offset += layouts_[i].size;
    }
    unbind();
    glBindBuffer(GL_ARRAY_BUFFER, 0);
  }
  void updateVBOData(void *data, unsigned int size) {
    if (!is_changed_) {
      std::printf(
          "[WARNING]: this VBO is not dynamic to change data frequently\n");
    }
    glBindBuffer(GL_ARRAY_BUFFER, VBOHandler_);
    glBufferSubData(GL_ARRAY_BUFFER, 0, size, data);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
  }
  void addEBOData(std::vector<unsigned int> &indices) {
    countEBO_ = indices.size();
    bind();
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBOHandler_);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices[0]) * indices.size(),
                 indices.data(), GL_STATIC_DRAW);
    unbind();
  }
  unsigned int getEBOCount() { return countEBO_; }
  void addAttributeLayout(const VBOAttributeConfig &config) {
    layouts_.push_back(config);
    step_ += config.size;
  }

private:
  unsigned int VAOHandler_;
  unsigned int VBOHandler_;
  unsigned int EBOHandler_;
  bool is_changed_ = false;

  std::vector<VBOAttributeConfig> layouts_;
  int step_ = 0;
  unsigned int countEBO_ = 0;
};
}; // namespace my