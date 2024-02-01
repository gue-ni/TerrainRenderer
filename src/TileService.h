#pragma once

#include <condition_variable>
#include <format>
#include <functional>
#include <iostream>
#include <mutex>
#include <queue>
#include <set>
#include <stack>
#include <string>
#include <thread>
#include <tuple>
#include <unordered_map>

#include "../gfx/image.h"
#include "TileUtils.h"

using namespace gfx;

template <typename T>
class ThreadedQueue
{
 public:
  ThreadedQueue() = default;

  ~ThreadedQueue() { stop(); }

  void push(const T& item)
  {
    {
      std::unique_lock lock(m_mutex);
      m_queue.push(item);
    }
    m_condition.notify_one();
  }

  void push(T&& item)
  {
    {
      std::unique_lock lock(m_mutex);
      m_queue.push(std::move(item));
    }
    m_condition.notify_one();
  }

  bool pop(T& item)
  {
    std::unique_lock lock(m_mutex);
    m_condition.wait(lock, [&]() { return !m_queue.empty() || m_stop; });

    if (m_stop) {
      return false;
    }

    item = std::move(m_queue.front());
    m_queue.pop();
    return true;
  }

  size_t size() const
  {
    std::unique_lock lock(m_mutex);
    return m_queue.size();
  }

  bool empty() const
  {
    std::unique_lock lock(m_mutex);
    return m_queue.empty();
  }

  void stop()
  {
    {
      std::unique_lock lock(m_mutex);
      m_stop = true;
    }
    m_condition.notify_all();
  }

 private:
  bool m_stop = false;
  std::queue<T> m_queue;
  std::mutex m_mutex;
  std::condition_variable m_condition;
};

class ThreadPool
{
 public:
  using Work = std::function<void(void)>;

  ThreadPool(size_t thread_count = std::thread::hardware_concurrency())
  {
    m_threads.reserve(thread_count);

    for (auto i = 0U; i < thread_count; ++i) {
      auto worker = [this]() {
        while (true) {
          Work work{};

          // std::cout << "Waiting for work...\n";
          m_queue.pop(work);

          if (!work) {
            break;
          }

          // std::cout << "Do some work...\n";
          work();
        }
      };

      m_threads.push_back(std::thread(worker));
    }
  }

  ~ThreadPool()
  {
    for (auto i = 0U; i < m_threads.size(); ++i) {
      m_queue.push(Work{});  // null item
    }

    for (auto& t : m_threads) {
      t.join();
    }
  }

  void assign_work(Work work) { m_queue.push(std::move(work)); }

 private:
  ThreadedQueue<Work> m_queue;
  std::vector<std::thread> m_threads;
};

enum UrlPattern {
  ZXY_Y_NORTH,
  ZXY_Y_SOUTH,
  ZYX_Y_NORTH,
  ZYX_Y_SOUTH,
};

class TileService
{
 public:
  TileService(const std::string& url, const UrlPattern& url_pattern, const std::string& filetype = "png");

  // If tile in cache, return tile. If not, request it for download and return nullptr
  Image* get_tile(const TileId&);

  // get image sync
  Image* get_tile_sync(const TileId&);

 private:
  const UrlPattern m_url_pattern;
  const std::string m_url, m_filetype;
  ThreadPool m_thread_pool;
  std::set<TileId> m_already_requested;
  std::unordered_map<TileId, std::unique_ptr<Image>> m_ram_cache;

  void request_tile(const TileId&);

  std::unique_ptr<Image> download_tile(const TileId&);

  std::string tile_url(const TileId&) const;
};
