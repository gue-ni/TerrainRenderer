#include "TileService.h"

#include <cpr/cpr.h>

#define LOG 0

TileService::TileService(const std::string& url, const UrlPattern& url_pattern, const std::string& filetype)
    : m_url(url), m_url_pattern(url_pattern), m_filetype(filetype), m_thread_pool(2)
{
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
}

Image* TileService::get_tile(const TileId& tile_id)
{
  std::string tile_id_str = tile_id.to_string();

  if (m_ram_cache.contains(tile_id_str)) {
    return m_ram_cache[tile_id_str].get();
  }

  if (!m_already_requested.contains(tile_id)) {
    request_tile(tile_id);
  }
  return nullptr;
}

Image* TileService::get_tile_sync(const TileId& tile_id)
{
  auto image = download_tile(tile_id);

  if (!image) {
    return nullptr;
  }

  auto tile_id_str = tile_id.to_string();
  m_ram_cache[tile_id_str] = std::move(image);
  return m_ram_cache[tile_id_str].get();
}

void TileService::request_tile(const TileId& tile_id)
{
  m_already_requested.insert(tile_id);

  m_thread_pool.assign_work([this, tile_id]() {

    auto image = download_tile(tile_id);

    if (image) {
      auto tile_id_str = tile_id.to_string();
      m_ram_cache[tile_id_str] = std::move(image);
    }
  });
}

std::unique_ptr<Image> TileService::download_tile(const TileId& tile_id)
{
  auto url = tile_url(tile_id);
  cpr::Response r = cpr::Get(cpr::Url{url});

  if (r.status_code != 200) {
    std::cerr << "Error " << r.status_code << " " << std::quoted(url) << std::endl;
    return nullptr;
  }

  std::cout << "GET " << std::quoted(url) << std::endl;

  auto image = std::make_unique<Image>();
  image->read_from_buffer(reinterpret_cast<unsigned char*>(r.text.data()), int(r.text.size()));

  if (!image->loaded()) {
    std::cerr << "Could not read " << tile_id << std::endl;
    return nullptr;
  }

  return image;
}
