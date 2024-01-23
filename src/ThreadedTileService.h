#pragma once

#include <condition_variable>
#include <iostream>
#include <mutex>
#include <queue>
#include <thread>

#include "TileService.h"

class Worker
{
 public:
  Worker();
  ~Worker();
  void request_tile(float lat, float lon, unsigned zoom);

 private:
};

class ThreadedTileService : public TileService
{
 public:
 private:
};
