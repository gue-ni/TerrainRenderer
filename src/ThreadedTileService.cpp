#include "ThreadedTileService.h"

#include <cpr/cpr.h>

bool contains() { return false; }

void ThreadedTileService::request_download(float lat, float lon, unsigned zoom)
{
  std::unique_lock<std::mutex> lock(m_mutex);

  TileId id = wms::tile_id(lat, lon, zoom);
  m_tiles_to_download.push(id);
  lock.unlock();
  m_condition.notify_one();

  m_already_requested.insert(id.to_string());
}

ThreadedTileService::~ThreadedTileService() { 
  m_thread.join(); 
}

void ThreadedTileService::start_worker_thread()
{
  m_thread = std::thread([this]() {
    while (true) {
      std::unique_lock<std::mutex> lock(m_mutex);
      m_condition.wait(lock, [this] { return !m_tiles_to_download.empty(); });
      auto tile_id = m_tiles_to_download.top();
      m_tiles_to_download.pop();
      lock.unlock();

      auto filename = tile_filename(tile_id.x, tile_id.y, tile_id.zoom);

      if (!std::filesystem::exists(filename)) {
        auto url = tile_url(tile_id.x, tile_id.y, tile_id.zoom);

        std::ofstream of(filename, std::ios::binary);
        cpr::Response r = cpr::Download(of, cpr::Url{url});

        if (r.status_code != 200) {
          std::cerr << "Could not download " << std::quoted(url) << std::endl;
          std::filesystem::remove(filename);
        } else {
          std::cout << "Download " << std::quoted(url) << std::endl;
        }

        assert(r.status_code == 200);
      }

      auto image = std::make_unique<Image>();
      image->read(filename);
      if (!image->loaded()) {
        std::cerr << "Could not read " << std::quoted(filename) << std::endl;
      }

      // assert(image->loaded());

      m_ram_cache[filename] = std::move(image);
    }
  });
}

Image* ThreadedTileService::get_tile(float lat, float lon, unsigned zoom)
{
  TileId tile_id = wms::tile_id(lat, lon, zoom);

  std::string tile_id_str = tile_filename(tile_id.x, tile_id.y, tile_id.zoom);

  if (m_ram_cache.contains(tile_id_str)) {
    return m_ram_cache[tile_id_str].get();
  } else {
    if (!m_already_requested.contains(tile_id.to_string())) {
      request_download(lat, lon, zoom);
    }
    return nullptr;
  }
}
