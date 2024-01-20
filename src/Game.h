#pragma once
#include <memory>

#include "../gfx/gfx.h"
#include "Window.h"

using namespace gfx;
using namespace gfx::gl;

class Game : public Window
{
 public:
  Game(size_t width, size_t height);
  void run() override;

 private:
  std::unique_ptr<ShaderProgram> m_shader{nullptr};

  void render();
};
