#include "ThreadedTileService.h"

#include <cpr/cpr.h>

void ThreadedTileService::request_download(const TileId& tile_id)
{
  std::unique_lock<std::mutex> lock(m_mutex);
  m_tiles_to_download.push(tile_id);
  // lock.unlock();
  m_condition.notify_one();
  m_already_requested.insert(tile_id);
}

ThreadedTileService::~ThreadedTileService()
{
  m_stop_thread = true;
  TileId null{};
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

      get_tile_sync(tile_id);
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
      request_download(tile_id);
    }
    return nullptr;
  }
}

Image* ThreadedTileService::get_tile_sync(const TileId& tile_id)
{
  auto filename = tile_filename(tile_id.x, tile_id.y, tile_id.zoom);

  auto url = tile_url(tile_id.x, tile_id.y, tile_id.zoom);

  cpr::Response r = cpr::Get(cpr::Url{url});

  if (r.status_code != 200) {
    std::cerr << "Could not download " << std::quoted(url) << std::endl;
  } else {
    std::cout << "Download " << std::quoted(url) << std::endl;
  }

  auto image = std::make_unique<Image>();
  image->read_from_buffer(reinterpret_cast<unsigned char*>(r.text.data()), int(r.text.size()));

  if (image->loaded()) {
    m_ram_cache[filename] = std::move(image);
    return m_ram_cache[filename].get();
  } else {
    std::cerr << "Could not read " << std::quoted(filename) << std::endl;
    return nullptr;
  }
}
