#include "ThreadedTileService.h"

#include <cpr/cpr.h>

#define LOG 1

void ThreadedTileService::request_download(const TileId& tile_id)
{
  std::unique_lock<std::mutex> lock(m_mutex);

  m_tiles_to_download.push(tile_id);
  // lock.unlock();

  m_condition.notify_one();
  // m_condition.notify_all();

  m_already_requested.insert(tile_id);
}

ThreadedTileService::~ThreadedTileService()
{
  m_stop_thread = true;
  TileId null;
  request_download(null);
  m_thread.join();
}

void ThreadedTileService::start_worker_thread()
{
  auto worker = [this]() {
    while (!m_stop_thread) {
      std::unique_lock<std::mutex> lock(m_mutex);
      m_condition.wait(lock, [this] { return !m_tiles_to_download.empty(); });
      auto& tile_id = m_tiles_to_download.top();
      m_tiles_to_download.pop();
      lock.unlock();

      auto filename = tile_filename(tile_id.x, tile_id.y, tile_id.zoom);

      if (!std::filesystem::exists(filename)) {
        auto url = tile_url(tile_id.x, tile_id.y, tile_id.zoom);

        std::ofstream of(filename, std::ios::binary);
        cpr::Response r = cpr::Download(of, cpr::Url{url});

        if (r.status_code != 200) {
          std::cerr << "Could not download " << std::quoted(url) << std::endl;
          std::error_code err;
          std::filesystem::remove(filename, err);
        } else {
#if LOG
          std::cout << "Download " << std::quoted(url) << std::endl;
#endif
        }
      }

      auto image = std::make_unique<Image>();
      image->read(filename);

      if (image->loaded()) {
        m_ram_cache[filename] = std::move(image);
      } else {
        std::cerr << "Could not read " << std::quoted(filename) << std::endl;
      }

      // std::cout << "Load " << std::quoted(tile_id.to_string()) << std::endl;
    }
  };

  m_thread = std::thread(worker);
}

Image* ThreadedTileService::get_tile(const TileId& tile_id)
{
  std::string tile_id_str = tile_filename(tile_id.x, tile_id.y, tile_id.zoom);

  if (m_ram_cache.contains(tile_id_str)) {
    return m_ram_cache[tile_id_str].get();
  } else {
    if (!m_already_requested.contains(tile_id)) {
      // std::cout << "Request " << std::quoted(tile_id.to_string()) << std::endl;
      request_download(tile_id);
    }
    return nullptr;
  }
}
