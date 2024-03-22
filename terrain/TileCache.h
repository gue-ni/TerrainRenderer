/*
  Loads WMS tiles from disk
  Tiles are 256x256 pixels

*/
#pragma once

#include <chrono>
#include <memory>
#include <string>
#include <unordered_map>

#include "../gfx/gfx.h"
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
  TileCache();

  Texture* tile_texture(const TileId&, const TileType&);

  Texture* tile_texture_sync(const TileId&, const TileType&);

  Texture* tile_texture_cached(const TileId&, const TileType&);

  float elevation(const Coordinate&);

 private:
  std::unordered_map<std::string, std::unique_ptr<Texture>> m_gpu_cache;
  TileService m_ortho_service, m_height_service;

  std::unique_ptr<Texture> create_texture(const Image& image);

  Image* request_image(const TileId&, const TileType&);
};
