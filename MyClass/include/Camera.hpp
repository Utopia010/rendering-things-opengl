#pragma once

#include <GLFW/glfw3.h>
#include <cmath>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/vec3.hpp>
#include <imgui.h>

namespace my {
static const glm::vec3 UP = glm::vec3{0.f, 1.0f, 0.0f};
static const glm::vec3 DOWN = glm::vec3{0.f, -1.0f, 0.0f};
static const glm::vec3 FRONT = glm::vec3{0.f, 0.0f, -1.0f};
static const glm::vec3 BACK = glm::vec3{0.f, 0.0f, 1.0f};
static const glm::vec3 RIGHT = glm::vec3{1.f, 0.0f, 0.0f};
static const glm::vec3 LEFT = glm::vec3{-1.f, 0.0f, 0.0f};

struct CameraConfig {
  float ratio = 16.0 / 9.0;
  float fovy = 45;
  glm::vec3 direction = FRONT;
  glm::vec3 position = glm::vec3{0.f, 0.f, 1.f};
  float pitch = 90.0f;
  float yaw = 270.f;
  float speedMove = 3.f;
  float mouseSensitivity = 2.f;
  float near = 0.1f, far = 100.f;
};

class Camera {
public:
  explicit Camera(CameraConfig config) : config_(std::move(config)) {}
  void update(GLFWwindow *win, bool is_covered) {
    if (!is_covered) {
      updateMove(win);
    }
    updateMVP();
  }

private:
  void updateMVP() {
    mvp_view_ =
        glm::lookAt(config_.position, config_.position + config_.direction, UP);
    mvp_project_ = glm::perspective(config_.fovy, config_.ratio, config_.near,
                                    config_.far);
  }
  void updateMove(GLFWwindow *win) {
    static double last_time = -1;
    if (last_time == -1) {
      last_time = glfwGetTime();
      return;
    }
    double delta_time = glfwGetTime() - last_time;
    last_time = glfwGetTime();

    // move
    glm::vec3 deltaMove{0.0f, 0.0f, 0.0f};
    if (glfwGetKey(win, GLFW_KEY_W) == GLFW_PRESS) {
      deltaMove += config_.direction;
    }
    if (glfwGetKey(win, GLFW_KEY_S) == GLFW_PRESS) {
      deltaMove -= config_.direction;
    }
    if (glfwGetKey(win, GLFW_KEY_A) == GLFW_PRESS) {
      deltaMove -= glm::cross(config_.direction, UP);
    }
    if (glfwGetKey(win, GLFW_KEY_D) == GLFW_PRESS) {
      deltaMove += glm::cross(config_.direction, UP);
    }
    if (glfwGetKey(win, GLFW_KEY_UP) == GLFW_PRESS) {
      deltaMove += UP;
    }
    if (glfwGetKey(win, GLFW_KEY_DOWN) == GLFW_PRESS) {
      deltaMove += DOWN;
    }

    if (std::abs(deltaMove.x) > 1e-6 || std::abs(deltaMove.y) > 1e-6 ||
        std::abs(deltaMove.z) > 1e-6) {
      deltaMove = glm::normalize(deltaMove) * static_cast<float>(delta_time) *
                  config_.speedMove;
      config_.position += deltaMove;
    }

    // cursor
    static bool has_pressed = false;
    static glm::vec2 last_pos{};
    if (glfwGetMouseButton(win, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
      double x, y;
      glfwGetCursorPos(win, &x, &y);
      if (!has_pressed) {
        has_pressed = true;
        last_pos.x = x, last_pos.y = y;
        return;
      }
      glm::vec2 delta_mouse(x - last_pos.x, last_pos.y - y);
      last_pos = glm::vec2(x, y);
      delta_mouse *= delta_time * config_.mouseSensitivity;
      config_.yaw += delta_mouse.x;
      config_.pitch += delta_mouse.y;
      config_.pitch = std::clamp(config_.pitch, 1.f, 179.f);

      config_.direction.x = glm::cos(glm::radians(config_.yaw));
      config_.direction.z = glm::sin(glm::radians(config_.yaw));
      config_.direction.y = glm::cos(glm::radians(180 - config_.pitch));
      config_.direction = glm::normalize(config_.direction);
    } else if (glfwGetMouseButton(win, GLFW_MOUSE_BUTTON_LEFT) ==
               GLFW_RELEASE) {
      has_pressed = false;
      return;
    }
  }

public:
  CameraConfig config_;
  glm::mat4 mvp_view_{1.f}, mvp_project_{1.f};
};

static const CameraConfig DefaultConfig{};

}; // namespace my