#include "ThreadedTileService.h"

#include <cpr/cpr.h>

void ThreadedTileService::request_tile(float lat, float lon, unsigned zoom)
{
  std::string url;  // TOOD

  std::unique_lock<std::mutex> lock(m_mutex);
  m_tasks.push(url);
  lock.unlock();
  m_condition.notify_one();
}

void ThreadedTileService::start_worker_thread()
{
  m_thread = std::thread([this]() {
    while (true) {
      std::unique_lock<std::mutex> lock(m_mutex);

      m_condition.wait(lock, [this] { return !m_tasks.empty(); });

      std::string filename = "tmp.bin";
      std::string url = m_tasks.front();
      m_tasks.pop();

      lock.unlock();

      //std::ofstream of(filename, std::ios::binary);
      //cpr::Response r = cpr::Download(of, cpr::Url{url});
      }

    });
}
