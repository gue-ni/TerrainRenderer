#pragma once
#include <glm/glm.hpp>

#include "../gfx/gfx.h"
#include "Chunk.h"
#include "QuadTree.h"
#include "TileCache.h"

using namespace gfx;
using namespace gfx::gl;

class TerrainRenderer
{
 public:
  TerrainRenderer(const glm::vec2& min, const glm::vec2& max);
  void render(const Camera& camera, const glm::vec2& center);

  bool wireframe{false};

 private:
  std::unique_ptr<ShaderProgram> m_shader{nullptr};

  Chunk m_debug_chunk;
  Bounds m_bounds;
  TileCache m_tile_cache;
};
