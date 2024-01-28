#pragma once

#include <condition_variable>
#include <format>
#include <iostream>
#include <mutex>
#include <queue>
#include <set>
#include <stack>
#include <string>
#include <thread>
#include <tuple>
#include <unordered_map>

#include "TileUtils.h"
#include "../gfx/gfx.h"
#include "../gfx/image.h"

using namespace gfx;

class ThreadedTileService
{
 public:
  enum UrlPattern {
    ZXY,
    ZXY_Y_SOUTH,
    ZYX,
    ZYX_Y_SOUTH,
  };

  ThreadedTileService(const std::string& url, const UrlPattern& url_pattern, const std::string& filetype = "png");
  ~ThreadedTileService();

  // Start seperate worker thread
  void start_worker_thread();

  // If tile in cache, return tile. If not, request it for download and return nullptr
  Image* get_tile(const TileId& tile_id);

  // get image sync
  Image* get_tile_sync(const TileId& tile_id);

 private:
  const UrlPattern m_url_pattern;
  const std::string m_url, m_filetype;
  bool m_stop_thread{false};
  std::thread m_thread;
  std::stack<TileId> m_tiles_to_download;
  std::mutex m_mutex;
  std::condition_variable m_condition;
  std::unordered_map<std::string, std::unique_ptr<Image>> m_ram_cache;
  std::set<TileId> m_already_requested;

  void request_download(const TileId&);

  std::string tile_url(unsigned x, unsigned y, unsigned zoom) const;

  std::string tile_filename(unsigned x, unsigned y, unsigned zoom) const;
};
