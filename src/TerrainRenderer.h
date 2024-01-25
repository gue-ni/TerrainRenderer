#pragma once
#include <glm/glm.hpp>

#include "../gfx/gfx.h"
#include "Chunk.h"
#include "QuadTree.h"
#include "TileCache.h"
#include "TileService.h"

using namespace gfx;
using namespace gfx::gl;

class TerrainRenderer
{
 public:
  TerrainRenderer(const glm::vec2& min, const glm::vec2& max);
  void render(const Camera& camera, const glm::vec2& center);
  Bounds bounds() const { return m_bounds; }

  bool wireframe{false};

 private:
  std::unique_ptr<ShaderProgram> m_shader{nullptr};
  const TileId m_root_tile;
  const Chunk m_chunk;
  const Bounds m_bounds;
  TileCache m_tile_cache;
  float m_height_scaling_factor;

  // convert from world coordinate in range m_bounds to [0, 1]
  glm::vec2 map_to_0_1(const glm::vec2& point);
};
