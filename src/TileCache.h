/*
  Loads WMS tiles from disk
  Tiles are 256x256 pixels

*/
#pragma once

#include <chrono>
#include <format>
#include <memory>
#include <numbers>
#include <string>
#include <tuple>
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

inline TileName to_tilename(float lat, float lon, unsigned zoom)
{
  unsigned x = wms::lon2tilex(lon, zoom);
  unsigned y = wms::lat2tiley(lat, zoom);
  return {.zoom = zoom, .x = x, .y = y};
}

// width of tile in meters
inline float tile_width(float lat, unsigned zoom)
{
  const float C = 40075016.686f;
  return std::abs(C * std::cos(lat) / (1 << zoom));
}

};  // namespace wms

enum TileType { ORTHO, HEIGHT };

using TimePoint = std::chrono::time_point<std::chrono::system_clock>;

struct CacheInfo {
  TimePoint last_accessed;
  void accessed() { last_accessed = std::chrono::system_clock::now(); }
};

class TileCache
{
 public:
  TileCache(const TileName& root_tile, unsigned max_zoom_level);

  Texture* get_debug_texture() { return m_debug_texture.get(); }

  // get the tile that contains point at a specific level of detail
  Texture* get_tile_texture(const glm::vec2& point, unsigned lod = 0, const TileType& tile_type = TileType::ORTHO);

  void invalidate_gpu_cache();

 private:
  const TileName m_root_tile;
  const unsigned m_max_zoom_level;

  TileService m_ortho_tile_service;
  TileService m_height_tile_service;

  std::unique_ptr<Texture> m_debug_texture{nullptr};

  std::unordered_map<std::string, std::tuple<CacheInfo, std::unique_ptr<Texture>>> m_gpu_cache;

  std::unordered_map<std::string, std::tuple<CacheInfo, std::unique_ptr<Image>>> m_ram_cache;  // TODO

  std::unique_ptr<Texture> load_texture_from_disk(float lat, float lon, unsigned zoom, const TileType& tile_type);
  Texture* load_texture_from_cache(float lat, float lon, unsigned zoom, const TileType& tile_type);
};
