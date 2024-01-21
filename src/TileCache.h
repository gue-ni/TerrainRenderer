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

namespace wms
{
constexpr float PI = 3.1415f;

inline int long2tilex(float lon, int z) { return (int)(floor((lon + 180.0f) / 360.0f * (1 << z))); }

inline int lat2tiley(float lat, int z)
{
  float latrad = lat * PI / 180.0f;
  return (int)(floor((1.0f - asinh(tan(latrad)) / PI) / 2.0f * (1 << z)));
}

inline float tilex2long(int x, int z) { return x / (float)(1 << z) * 360.0f - 180.0f; }

inline float tiley2lat(int y, int z)
{
  float n = PI - 2.0f * PI * y / (float)(1 << z);
  return 180.0f / PI * atan(0.5f * (exp(n) - exp(-n)));
}

};  // namespace wms

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
