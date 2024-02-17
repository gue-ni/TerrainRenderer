#pragma once

#include <iostream>
#include <set>
#include <string>
#include <unordered_map>

#include "../gfx/image.h"
#include "Threading.h"
#include "TileUtils.h"

#ifndef CACHE_ON_DISK
#define CACHE_ON_DISK true
#endif

using namespace gfx;

enum UrlPattern {
  ZXY_Y_NORTH,
  ZXY_Y_SOUTH,
  ZYX_Y_NORTH,
  ZYX_Y_SOUTH,
};

class TileService
{
 public:
  TileService(const std::string& url, const UrlPattern& url_pattern, const std::string& filetype = "png",
              const std::string& cache_dir = "");

  Image* get_tile_cached(const TileId&);

  // If tile in cache, return tile. If not, request it for download and return nullptr.
  Image* get_tile(const TileId&);

  // Download tile and return it.
  Image* get_tile_sync(const TileId&);

 private:
  const UrlPattern m_url_pattern;
  const std::string m_url, m_filetype, m_cache_dir;
  ThreadPool m_thread_pool;
  std::set<TileId> m_already_requested;
  std::unordered_map<TileId, std::unique_ptr<Image>> m_ram_cache;

  void request_tile(const TileId&);

  std::unique_ptr<Image> download_tile(const TileId&);

  std::string tile_url(const TileId&) const;

  std::string tile_filename(const TileId&) const;

  void save_to_disk(const TileId&, const Image*) const;

  std::unique_ptr<Image> load_from_disk(const TileId&) const;

  bool is_saved_on_disk(const TileId&) const;
};
