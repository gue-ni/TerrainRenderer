#include "TileService.h"

#include <cpr/cpr.h>
#include <fmt/core.h>

#include <filesystem>

#define LOG_REQUESTS  true
#define CACHE_ON_DISK true
#define NUM_THREADS   3

inline std::ostream& operator<<(std::ostream& os, const TileId& tile)
{
  return os << tile.zoom << "-" << tile.x << "-" << tile.y;
}

TileService::TileService(const std::string& url, const UrlPattern& url_pattern, const std::string& filetype,
                         const std::string& dir)
    : m_url(url), m_url_pattern(url_pattern), m_filetype(filetype), m_thread_pool(NUM_THREADS), m_cache_dir(dir)
{
#if CACHE_ON_DISK
  if (!std::filesystem::exists(m_cache_dir)) {
    std::filesystem::create_directories(m_cache_dir);
  }
#endif
}

std::string TileService::tile_url(const TileId& tile) const
{
  const unsigned num_y_tiles = (1 << tile.zoom);

  switch (m_url_pattern) {
    case ZXY_Y_NORTH: {
      return fmt::format("{}/{}/{}/{}{}", m_url, tile.zoom, tile.x, (num_y_tiles - tile.y - 1), m_filetype);
    }
    case ZYX_Y_NORTH: {
      return fmt::format("{}/{}/{}/{}{}", m_url, tile.zoom, (num_y_tiles - tile.y - 1), tile.x, m_filetype);
    }
    case ZYX_Y_SOUTH: {
      return fmt::format("{}/{}/{}/{}{}", m_url, tile.zoom, tile.y, tile.x, m_filetype);
    }
    case ZXY_Y_SOUTH: {
      return fmt::format("{}/{}/{}/{}{}", m_url, tile.zoom, tile.x, tile.y, m_filetype);
    }
    default:
      assert(false);
      return "";
  }
}

std::string TileService::tile_filename(const TileId& tile) const
{
  return fmt::format("{}/{}.png", m_cache_dir, tile.to_string());
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

  auto tile_request = [this, tile]() {
    auto image = download_tile(tile);
    if (image) m_ram_cache[tile] = std::move(image);
  };

  m_thread_pool.assign_work(tile_request);
}

std::unique_ptr<Image> TileService::download_tile(const TileId& tile)
{
#if CACHE_ON_DISK
  if (is_saved_on_disk(tile)) {
    auto image = load_from_disk(tile);

    if (image) {
#if LOG_REQUESTS
      std::cout << "Load from disk: " << m_cache_dir << " " << tile << "\n";
#endif
      return image;
    }
  }
#endif

  auto url = tile_url(tile);
  cpr::Response r = cpr::Get(cpr::Url{url});

  if (r.status_code != 200) {
    std::cerr << "Error " << r.status_code << " " << std::quoted(url) << "\n";
    return nullptr;
  }

#if LOG_REQUESTS
  std::cout << "Load from web: " << tile << "\n";
#endif

  auto image = std::make_unique<Image>();
  image->read_from_buffer(reinterpret_cast<unsigned char*>(r.text.data()), int(r.text.size()));

  if (!image->loaded()) {
    std::cerr << "Could not read " << tile << "\n";
    return nullptr;
  }

#if CACHE_ON_DISK
  save_to_disk(tile, image.get());
#endif

  return image;
}

void TileService::save_to_disk(const TileId& tile, const Image* image) const
{
  assert(image);
  image->write(tile_filename(tile));
}

std::unique_ptr<Image> TileService::load_from_disk(const TileId& tile) const
{
  auto image = std::make_unique<Image>();
  image->read(tile_filename(tile));

  if (!image->loaded()) {
    return nullptr;
  }

  return image;
}

bool TileService::is_saved_on_disk(const TileId& tile) const { return std::filesystem::exists(tile_filename(tile)); }
