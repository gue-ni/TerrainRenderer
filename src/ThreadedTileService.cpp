#include "ThreadedTileService.h"

#include <cpr/cpr.h>

void ThreadedTileService::request_download(float lat, float lon, unsigned zoom)
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

Image* ThreadedTileService::get_tile(float lat, float lon, unsigned zoom)
{
  TileName tile_id = wms::to_tilename(lat, lon, zoom);
  std::string tile_id_str = tile_id.to_string();

  if (m_ram_cache.contains(tile_id_str)) {
    return m_ram_cache[tile_id_str].get();
  } else {
    request_download(lat, lon, zoom);
    return nullptr;
  }
}