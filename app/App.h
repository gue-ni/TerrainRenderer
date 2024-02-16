#pragma once
#include <memory>

#define GLM_ENABLE_EXPERIMENTAL
#include "../gfx/gfx.h"
#include "Clock.h"
#include "TerrainRenderer.h"
#include "Window.h"

using namespace gfx;
using namespace gfx::gl;

class FirstPersonCamera : public Camera
{
 public:
  float pitch = 0.0f;
  float yaw = 0.0f;

  FirstPersonCamera() : Camera() { update(); }

  void handle_input(const SDL_Event& event)
  {
    const float sensitivity = 0.20f;
    float delta_yaw = static_cast<float>(event.motion.xrel) * sensitivity;
    float delta_pitch = static_cast<float>(event.motion.yrel) * sensitivity;

    yaw += delta_yaw;
    pitch = glm::clamp(pitch - delta_pitch, -89.0f, 89.0f);

    update();
  }

  void update()
  {
    glm::vec3 world_up = {0.0f, 1.0f, 0.0f};
    glm::vec3 forward = vector_from_spherical(glm::radians(pitch), glm::radians(yaw));
    glm::vec3 right = glm::normalize(glm::cross(forward, world_up));
    glm::vec3 up = glm::normalize(glm::cross(right, forward));
    glm::vec3 position = local_position();

    set_local_transform(glm::inverse(glm::lookAt(position, position + forward, up)));
  }
};

class App : public Window
{
 public:
  App(size_t width, size_t height);
  void run();

 private:
  bool m_quit{false};
  bool m_paused{false};
  bool m_mousedown{false};
  float m_speed{100.0f};
  FirstPersonCamera m_camera;
  TerrainRenderer m_terrain;
  Clock m_clock;

  void read_input(float dt);
  void update(float dt);
  void render(float dt);
  void render_terrain();
  void render_ui();
};
