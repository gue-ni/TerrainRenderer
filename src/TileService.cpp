#include "TileService.h"

#include <cpr/cpr.h>

TileService::TileService(const std::string& url, const UrlPattern& url_pattern, const std::string& filetype)
    : m_url(url), m_url_pattern(url_pattern), m_filetype(filetype)
{
}

TileService::~TileService()
{
  m_stop_thread = true;
  TileId null{};
  request_download(null);
  m_thread.join();
}

std::string TileService::tile_url(const TileId& tile_id) const
{
  std::string url;
  unsigned zoom = tile_id.zoom, x = tile_id.x, y = tile_id.y;
  const unsigned num_y_tiles = (1 << zoom);

  switch (m_url_pattern) {
    case ZXY_Y_NORTH: {
      url = std::format("{}/{}/{}/{}{}", m_url, zoom, x, (num_y_tiles - y - 1), m_filetype);
      break;
    }
    case ZYX_Y_NORTH: {
      url = std::format("{}/{}/{}/{}{}", m_url, zoom, (num_y_tiles - y - 1), x, m_filetype);
      break;
    }
    case ZYX_Y_SOUTH: {
      url = std::format("{}/{}/{}/{}{}", m_url, zoom, y, x, m_filetype);
      break;
    }
    case ZXY_Y_SOUTH: {
      url = std::format("{}/{}/{}/{}{}", m_url, zoom, x, y, m_filetype);
      break;
    }
    default:
      assert(false);
  }

  return url;
}

void TileService::start_worker_thread()
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

Image* TileService::get_tile(const TileId& tile_id)
{
  std::string tile_id_str = tile_id.to_string();

  if (m_ram_cache.contains(tile_id_str)) {
    return m_ram_cache[tile_id_str].get();
  }

  if (!m_already_requested.contains(tile_id)) {
    request_download(tile_id);
  }
  return nullptr;
}

Image* TileService::get_tile_sync(const TileId& tile_id)
{
  auto url = tile_url(tile_id);
  auto tile_id_str = tile_id.to_string();

  cpr::Response r = cpr::Get(cpr::Url{url});

  if (r.status_code != 200) {
    std::cerr << "Could not download " << std::quoted(url) << std::endl;
  } else {
    std::cout << "Download " << std::quoted(url) << std::endl;
  }

  auto image = std::make_unique<Image>();
  image->read_from_buffer(reinterpret_cast<unsigned char*>(r.text.data()), int(r.text.size()));

  if (image->loaded()) {
    m_ram_cache[tile_id_str] = std::move(image);
    return m_ram_cache[tile_id_str].get();
  } else {
    std::cerr << "Could not read " << tile_id << std::endl;
    return nullptr;
  }
}

void TileService::request_download(const TileId& tile_id)
{
  std::unique_lock<std::mutex> lock(m_mutex);
  m_tiles_to_download.push(tile_id);
  m_condition.notify_one();
  m_already_requested.insert(tile_id);
}
