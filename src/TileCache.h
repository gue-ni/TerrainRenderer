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
#include "TileService.h"
#include "TileUtils.h"

using namespace gfx;
using namespace gfx::gl;

enum TileType : size_t { ORTHO = 0, HEIGHT = 1 };

using TimePoint = std::chrono::time_point<std::chrono::system_clock>;

struct CacheInfo {
  TimePoint last_accessed;
  void accessed() { last_accessed = std::chrono::system_clock::now(); }
};

class TileCache
{
 public:
  TileCache(const TileId& root_tile, unsigned max_zoom_level);

  Texture* tile_texture(const TileId&, const TileType&);

  Texture* tile_texture_sync(const TileId&, const TileType&);

  Texture* tile_texture_cached(const TileId&, const TileType&);

  float terrain_elevation(const Coordinate&);

  void invalidate_gpu_cache();

  Coordinate lat_lon(const glm::vec2& point);

  TileId tile_id(Coordinate& coord, unsigned lod_offset_from_root);

 private:
  const TileId m_root_tile;
  const unsigned m_max_zoom_level;
  const Coordinate m_min_coord, m_max_coord;
  std::unique_ptr<Texture> m_debug_texture{nullptr};
  std::unordered_map<std::string, std::unique_ptr<Texture>> m_gpu_cache;
  TileService m_ortho_service;
  TileService m_height_service;

  std::unique_ptr<Texture> create_texture(const Image& image);

  Image* request_image(const TileId&, const TileType&);
};
