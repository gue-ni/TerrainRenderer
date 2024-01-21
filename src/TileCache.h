/*
  Loads WMS tiles from disk
  Tiles are 256x256 pixels

*/
#pragma once

#include <format>
#include <memory>
#include <numbers>
#include <string>
#include <unordered_map>

#include "../gfx/gfx.h"
#include "QuadTree.h"
#include "TileProvider.h"

using namespace gfx;
using namespace gfx::gl;

struct TileName {
  unsigned zoom, x, y;

  std::string to_string() const { return std::format("{}/{}/{}", zoom, x, y); }
};

struct Coordinate {
  float lat, lon;
};

namespace wms
{
constexpr float PI = std::numbers::pi_v<float>;

inline int lon2tilex(float lon, int z) { return (int)(floor((lon + 180.0f) / 360.0f * (1 << z))); }

inline int lat2tiley(float lat, int z)
{
  float latrad = lat * PI / 180.0f;
  return (int)(floor((1.0f - asinh(tan(latrad)) / PI) / 2.0f * (1 << z)));
}

inline float tilex2lon(int x, int z) { return x / (float)(1 << z) * 360.0f - 180.0f; }

inline float tiley2lat(int y, int z)
{
  float n = PI - 2.0f * PI * y / (float)(1 << z);
  return 180.0f / PI * atan(0.5f * (exp(n) - exp(-n)));
}

inline glm::vec2 tile2lat_lon(int x, int y, int z) { return glm::vec2(tilex2lon(x, z), tiley2lat(y, z)); }

};  // namespace wms

class TileCache
{
 public:
  TileCache(const glm::vec2& min, const glm::vec2& max);
  TileCache(const glm::vec2& min, const glm::vec2& max, const TileName& root_tile, unsigned max_zoom_level);

  Texture* get_debug_texture() { return m_debug_texture.get(); }

  // get the tile that contains point at a specific level of detail
  Texture* get_tile_texture(const glm::vec2& point, unsigned lod = 0);

 private:
  const glm::vec2 m_min, m_max;
  const TileName m_root_tile;
  const unsigned m_max_zoom_level;

  LocalTileProvider m_provider;  // TODO: make this generic

  std::unique_ptr<Texture> m_debug_texture{nullptr};
  std::unordered_map<std::string, std::unique_ptr<Texture>> m_cache;

  std::unique_ptr<Texture> load_texture_from_disk(const TileName&);
  Texture* load_texture_from_cache(const TileName&);
};
