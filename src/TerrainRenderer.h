#pragma once
#include <glm/glm.hpp>

#include "../gfx/gfx.h"
#include "Chunk.h"
#include "QuadTree.h"

using namespace gfx;
using namespace gfx::gl;

class TerrainRenderer
{
 public:
  TerrainRenderer();
  void render(const Camera& camera, const glm::vec2& center);
  void set_wireframe(bool mode) { m_wireframe = mode; }

 private:
  bool m_wireframe{true};
  std::unique_ptr<ShaderProgram> m_shader{nullptr};

  Chunk m_debug_chunk;
  Bounds m_bounds;
};
