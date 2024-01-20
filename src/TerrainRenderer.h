#pragma once
#include <glm/glm.hpp>

#include "../gfx/gfx.h"

#include "Chunk.h"

using namespace gfx::gl;

class TerrainRenderer
{
 public:
  TerrainRenderer();
  void render(const glm::vec2& center);

 private:
  bool m_wireframe{true};
  std::unique_ptr<ShaderProgram> m_shader{nullptr};

  Chunk m_debug_chunk;
};
