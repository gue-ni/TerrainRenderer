#include "TileService.h"

#include <cpr/cpr.h>

#define LOG 0

TileService::TileService(const std::string& url, const UrlPattern& url_pattern, const std::string& filetype)
    : m_url(url), m_url_pattern(url_pattern), m_filetype(filetype), m_thread_pool(3)
{
}

std::string TileService::tile_url(const TileId& tile) const
{
  const unsigned num_y_tiles = (1 << tile.zoom);

  switch (m_url_pattern) {
    case ZXY_Y_NORTH: {
      return std::format("{}/{}/{}/{}{}", m_url, tile.zoom, tile.x, (num_y_tiles - tile.y - 1), m_filetype);
    }
    case ZYX_Y_NORTH: {
      return std::format("{}/{}/{}/{}{}", m_url, tile.zoom, (num_y_tiles - tile.y - 1), tile.x, m_filetype);
    }
    case ZYX_Y_SOUTH: {
      return std::format("{}/{}/{}/{}{}", m_url, tile.zoom, tile.y, tile.x, m_filetype);
    }
    case ZXY_Y_SOUTH: {
      return std::format("{}/{}/{}/{}{}", m_url, tile.zoom, tile.x, tile.y, m_filetype);
    }
    default:
      assert(false);
      return "";
  }
}

Image* TileService::get_tile(const TileId& tile)
{
  if (m_ram_cache.contains(tile)) {
    return m_ram_cache[tile].get();
  }

  if (!m_already_requested.contains(tile)) {
    request_tile(tile);
  }
  return nullptr;
}

Image* TileService::get_tile_sync(const TileId& tile)
{
  if (m_ram_cache.contains(tile)) {
    return m_ram_cache[tile].get();
  }

  auto image = download_tile(tile);

  if (!image) {
    return nullptr;
  }

  m_ram_cache[tile] = std::move(image);
  return m_ram_cache[tile].get();
}

void TileService::request_tile(const TileId& tile)
{
  m_already_requested.insert(tile);

  m_thread_pool.assign_work([this, tile]() {
    auto image = download_tile(tile);
    if (image) m_ram_cache[tile] = std::move(image);
  });
}

std::unique_ptr<Image> TileService::download_tile(const TileId& tile)
{
  auto url = tile_url(tile);
  cpr::Response r = cpr::Get(cpr::Url{url});

  if (r.status_code != 200) {
    std::cerr << "Error " << r.status_code << " " << std::quoted(url) << std::endl;
    return nullptr;
  }

  std::cout << "GET " << std::quoted(url) << std::endl;

  auto image = std::make_unique<Image>();
  image->read_from_buffer(reinterpret_cast<unsigned char*>(r.text.data()), int(r.text.size()));

  if (!image->loaded()) {
    std::cerr << "Could not read " << tile << std::endl;
    return nullptr;
  }

  return image;
}

void TileService::save_local_copy(const TileId& tile) const {}
