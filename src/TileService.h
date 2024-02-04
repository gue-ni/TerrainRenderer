#pragma once

#include <format>
#include <iostream>
#include <set>
#include <string>
#include <unordered_map>

#include "../gfx/image.h"
#include "Threading.h"
#include "TileUtils.h"

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
              const std::string& dir = "");

  // If tile in cache, return tile. If not, request it for download and return nullptr.
  Image* get_tile(const TileId&);

  // Download tile and return it.
  Image* get_tile_sync(const TileId&);

  inline void clear_pending_downloads()
  {
    m_already_requested = {};
    m_thread_pool.clear_queue();
  }

 private:
  const UrlPattern m_url_pattern;
  const std::string m_url, m_filetype, m_dir;
  ThreadPool m_thread_pool;
  std::set<TileId> m_already_requested;
  std::unordered_map<TileId, std::unique_ptr<Image>> m_ram_cache;

  void request_tile(const TileId&);

  std::unique_ptr<Image> download_tile(const TileId&);

  std::string tile_url(const TileId&) const;

  void save_local_copy(const TileId&, const Image*) const;
};
