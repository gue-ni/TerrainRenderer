#pragma once
#include <memory>

#include "../gfx/gfx.h"
#include "TerrainRenderer.h"
#include "Window.h"
#include "Clock.h"

using namespace gfx;
using namespace gfx::gl;

class FirstPersonCamera : public Camera
{
 public:
  float pitch{};
  float yaw{90.0f};
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
  Clock m_clock;

  glm::vec2 m_last_click_location{};

  void read_input(float dt);
  void update(float dt);
  void render(float dt);
};
