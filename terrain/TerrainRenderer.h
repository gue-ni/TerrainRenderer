#pragma once
#include <glm/glm.hpp>

#include "../gfx/gfx.h"
#include "Chunk.h"
#include "Common.h"
#include "Cube.h"
#include "QuadTree.h"
#include "TileCache.h"

using namespace gfx;
using namespace gfx::gl;

class TerrainRenderer
{
 public:
  TerrainRenderer(const TileId& root_tile, unsigned max_zoom_level_range, const Bounds<glm::vec2>& bounds);

  void render(const Camera& camera, const glm::vec2& center, float altitude = 0.0f);

  // get terrain elevation in meters
  float terrain_elevation(const glm::vec2&);

  float altitude_over_terrain(const glm::vec2&, float altitude);

  // relation of terrain unit to meters
  // divide to go from game coordinates to meters
  // multiply to go from meters to game
  inline float scaling_factor() const { return m_terrain_scaling_factor; }

  inline Bounds<glm::vec2> bounds() const { return m_bounds; }

  inline TileId root_tile() const { return m_root_tile; }

  Coordinate point_to_coordinate(const glm::vec2&) const;

  glm::vec2 coordinate_to_point(const Coordinate&) const;

  bool wireframe{false};
  bool intersect_terrain{false};
  bool debug_view{false};
  bool manual_zoom{false};
  float fog_far{2000.0f};
  float fog_density{0.25f};
  int min_zoom, max_zoom;

 private:
  const std::unique_ptr<ShaderProgram> m_shader, m_sky_shader;
  const TileId m_root_tile;
  const Chunk m_chunk;
  const Cube m_sky_box;
  const Bounds<glm::vec2> m_bounds;
  const Bounds<Coordinate> m_coord_bounds;
  const int m_max_zoom_level_range;
  TileCache m_tile_cache;
  float m_height_scaling_factor;
  float m_terrain_scaling_factor;

  void calculate_zoom_levels(const glm::vec2& center, float altitude);

  TileId tile_id_from_node(Node*) const;

  Texture* find_cached_lower_zoom_parent(Node* node, Bounds<glm::vec2>& uv, const TileType&);
};
