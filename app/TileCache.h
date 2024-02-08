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

#include "../gfx/gl.h"
#include "../gfx/image.h"
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
  TileCache(const TileId& root_tile);

  Texture* tile_texture(const TileId&, const TileType&);

  Texture* tile_texture_sync(const TileId&, const TileType&);

  Texture* tile_texture_cached(const TileId&, const TileType&);

  float terrain_elevation(const Coordinate&);

  void invalidate_gpu_cache();

  void clear_pending()
  {
    m_ortho_service.clear_pending_downloads();
    m_height_service.clear_pending_downloads();
  }

 private:
  const TileId m_root_tile;
  std::unordered_map<std::string, std::unique_ptr<Texture>> m_gpu_cache;
  TileService m_ortho_service;
  TileService m_height_service;

  std::unique_ptr<Texture> create_texture(const Image& image);

  Image* request_image(const TileId&, const TileType&);
};
