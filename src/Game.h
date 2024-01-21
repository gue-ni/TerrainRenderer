#pragma once
#include <memory>

#include "../gfx/gfx.h"
#include "TerrainRenderer.h"
#include "Window.h"

using namespace gfx;
using namespace gfx::gl;

class FirstPersonCamera : public Camera
{
 public:
  float pitch = M_PI / 2;
  float yaw = M_PI / 2;
};

class Game : public Window
{
 public:
  Game(size_t width, size_t height);
  void run();

 private:
  bool m_quit{false};
  FirstPersonCamera m_camera;
  TerrainRenderer m_terrain_renderer;

  glm::vec2 m_last_click_location{};

  void read_input();
  void update(float dt);
  void render();
};
