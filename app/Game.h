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
  float pitch{};
  float yaw{-90.0f};
};

class Game : public Window
{
 public:
  Game(size_t width, size_t height);
  void run();

 private:
  bool m_quit{false};
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
