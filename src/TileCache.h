/*
  Loads WMS tiles from disk
  Tiles are 256x256 pixels

*/
#pragma once

#include <memory>
#include <string>
#include <unordered_map>

#include "../gfx/gfx.h"

using namespace gfx;
using namespace gfx::gl;

struct TileName {
  unsigned zoom, x, y;
};

class TileCache
{
 public:
  TileCache(const glm::vec2& min, const glm::vec2& max);
  Texture* get_tile_texture() { return m_debug_texture.get(); }
  Texture* get_tile_texture(const glm::vec2& min, const glm::vec2& max, unsigned lod = 0);

 private:
  const glm::vec2 m_min, m_max;
  const std::string m_tile_root_path = "C:/Users/jakob/Pictures/tiles";
  const TileName m_root_tile{.zoom = 10, .x = 536, .y = 356};
  std::unique_ptr<Texture> m_debug_texture{nullptr};

  std::unique_ptr<Texture> load_texture(const TileName& tile_name);
};
