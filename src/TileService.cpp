#include "TileService.h"

#include <cpr/cpr.h>

#define LOG 0

TileService::TileService(const std::string& url, const UrlPattern& url_pattern, const std::string& filetype)
    : m_url(url), m_url_pattern(url_pattern), m_filetype(filetype)
{
}

TileService::~TileService()
{
  m_stop_thread = true;

  for (unsigned i = 0; i < m_num_threads; ++i) {
    request_download(TileId(0, 0, i));
  }

  std::for_each(m_threads.begin(), m_threads.end(), [](auto& t) { t.join(); });
}

std::string TileService::tile_url(const TileId& t) const
{
  const unsigned num_y_tiles = (1 << t.zoom);

  switch (m_url_pattern) {
    case ZXY_Y_NORTH: {
      return std::format("{}/{}/{}/{}{}", m_url, t.zoom, t.x, (num_y_tiles - t.y - 1), m_filetype);
    }
    case ZYX_Y_NORTH: {
      return std::format("{}/{}/{}/{}{}", m_url, t.zoom, (num_y_tiles - t.y - 1), t.x, m_filetype);
    }
    case ZYX_Y_SOUTH: {
      return std::format("{}/{}/{}/{}{}", m_url, t.zoom, t.y, t.x, m_filetype);
    }
    case ZXY_Y_SOUTH: {
      return std::format("{}/{}/{}/{}{}", m_url, t.zoom, t.x, t.y, m_filetype);
    }
    default:
      assert(false);
      return "";
  }
}

void TileService::start_worker_threads()
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

  for (auto i = 0; i < m_num_threads; ++i) {
    m_threads.emplace_back(std::thread(worker));
  }
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
  auto tile_id_str_1 = tile_id.to_string();

  cpr::Response r = cpr::Get(cpr::Url{url});

  if (r.status_code != 200) {
    std::cerr << "Error " << r.status_code << " " << std::quoted(url) << std::endl;
    return nullptr;
  }

#if LOG
  std::cout << "GET " << std::quoted(url) << std::endl;
#endif

  auto image = std::make_unique<Image>();
  image->read_from_buffer(reinterpret_cast<unsigned char*>(r.text.data()), int(r.text.size()));

  if (!image->loaded()) {
    std::cerr << "Could not read " << tile_id << std::endl;
    return nullptr;
  }

  auto tile_id_str_2 = tile_id.to_string();

  // What???
  // TODO: fix this, probably some problem with thread sync
  // assert(tile_id_str_1 == tile_id_str_2);

  if (!(tile_id_str_1 == tile_id_str_2)) {
    std::cout << tile_id << ": " << std::quoted(tile_id_str_1) << " != " << std::quoted(tile_id_str_2) << std::endl;
  }

  m_ram_cache[tile_id_str_1] = std::move(image);
  return m_ram_cache[tile_id_str_1].get();
}

void TileService::request_download(const TileId& tile_id)
{
  std::unique_lock<std::mutex> lock(m_mutex);
  m_tiles_to_download.push(tile_id);
  m_condition.notify_one();
  m_already_requested.insert(tile_id);
}
