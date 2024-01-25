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
#include "ThreadedTileService.h"
#include "TileService.h"
#include "TileUtils.h"

using namespace gfx;
using namespace gfx::gl;

#define MULTITHREADING 1

enum TileType { ORTHO, HEIGHT };

using TimePoint = std::chrono::time_point<std::chrono::system_clock>;

struct CacheInfo {
  TimePoint last_accessed;
  void accessed() { last_accessed = std::chrono::system_clock::now(); }
};

class TileCache
{
 public:
  TileCache(const TileId& root_tile, unsigned max_zoom_level);

  Texture* get_debug_texture() { return m_debug_texture.get(); }

  // get the tile that contains point at a specific level of detail
  // point is in range [0, 1] relative to the root tile
  Texture* get_tile_texture(const glm::vec2& point, unsigned lod = 0, const TileType& tile_type = TileType::ORTHO);

  Texture* get_tile_texture(const TileId& tile, const TileType& tile_type);

  Texture* get_cached_texture(const glm::vec2& point, unsigned lod = 0, const TileType& tile_type = TileType::ORTHO);

  void invalidate_gpu_cache();

  Coordinate lat_lon(const glm::vec2& point, unsigned lod);

  TileId tile_id(Coordinate& coord, unsigned zoom);

 private:
  const TileId m_root_tile;
  const unsigned m_max_zoom_level;
  Coordinate m_min_coord, m_max_coord;

#if MULTITHREADING
  ThreadedTileService m_ortho_tile_service;
  ThreadedTileService m_height_tile_service;
#else
  TileService m_ortho_tile_service;
  TileService m_height_tile_service;
#endif

  std::unique_ptr<Texture> m_debug_texture{nullptr};

  std::unordered_map<std::string, std::unique_ptr<Texture>> m_gpu_cache;

  // Texture* load_texture(float lat, float lon, unsigned zoom, const TileType& tile_type);

  Texture* load_texture(const TileId& tile_id, const TileType& tile_type);

  std::unique_ptr<Texture> create_texture(const Image& image);

  Image* request_image(const TileId&, const TileType&);
};
