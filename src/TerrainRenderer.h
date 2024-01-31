#pragma once
#include <glm/glm.hpp>

#include "../gfx/gfx.h"
#include "Chunk.h"
#include "Common.h"
#include "QuadTree.h"
#include "TileCache.h"

using namespace gfx;
using namespace gfx::gl;

class TerrainRenderer
{
 public:
  TerrainRenderer(const TileId& root_tile, unsigned zoom_levels, const Bounds<glm::vec2>& bounds);

  void render(const Camera& camera, const glm::vec2& center);

  Bounds<glm::vec2> bounds() const { return m_bounds; }

  // Get terrain elevation at point.
  float terrain_elevation(const glm::vec2& point);

  inline unsigned zoom_levels() const { return m_zoom_levels; }

  // Set range of zoom levels.
  inline void set_zoom_levels(unsigned zoom_levels) { m_zoom_levels = std::clamp(zoom_levels, 1U, 7U); }

  bool wireframe{false};

 private:
  const std::unique_ptr<ShaderProgram> m_shader{nullptr};
  const TileId m_root_tile;
  const Chunk m_chunk;  // terrain chunk geometry
  const Bounds<glm::vec2> m_bounds;
  unsigned m_zoom_levels;
  TileCache m_tile_cache;
  float m_height_scaling_factor;

  // convert from world coordinate in range m_bounds to [0, 1]
  glm::vec2 map_to_0_1(const glm::vec2& point) const;

  TileId tile_id_from_node(Node*) const;

  Texture* find_cached_lower_lod_parent(Node* node, glm::vec2& uv_min, glm::vec2& uv_max, const TileType&);
};
