#pragma once

#include <condition_variable>
#include <iostream>
#include <mutex>
#include <queue>
#include <thread>

#include "TileService.h"



class ThreadedTileService : public TileService
{
 public:
  ~ThreadedTileService();
  void request_tile(float lat, float lon, unsigned zoom);
  void start_worker_thread();

 private:
      std::thread m_thread;
    std::queue<std::string> m_tasks; // queue of urls to download
    std::mutex m_mutex;
    std::condition_variable m_condition;
};
