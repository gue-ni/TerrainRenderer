#pragma once

#include <condition_variable>
#include <iostream>
#include <mutex>
#include <queue>
#include <set>
#include <stack>
#include <thread>
#include <tuple>
#include <unordered_map>

#include "TileService.h"
#include "TileUtils.h"

class ThreadedTileService : public TileService
{
 public:
  using TileService::TileService;
  ~ThreadedTileService();

  // Start seperate worker thread
  void start_worker_thread();

  // If tile in cache, return tile. If not, request it for download and return nullptr
  Image* get_tile(const TileId& tile_id) override;

 private:
  bool m_stop_thread{false};
  std::thread m_thread;
  std::stack<TileId> m_tiles_to_download;
  std::mutex m_mutex;
  std::condition_variable m_condition;
  std::unordered_map<std::string, std::unique_ptr<Image>> m_ram_cache;
  std::set<TileId> m_already_requested;

  void request_download(const TileId&);
};
