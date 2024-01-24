#include "TileService.h"

#include <cpr/cpr.h>

#include <filesystem>
#include <format>

TileService::TileService(const std::string& url, const UrlPattern& url_pattern, const std::string& filetype)
    : m_url(url), m_url_pattern(url_pattern), m_filetype(filetype)
{
  if (!std::filesystem::exists(m_cache_location)) {
    std::filesystem::create_directory(m_cache_location);
  }
}

std::string TileService::tile_filename(unsigned x, unsigned y, unsigned zoom) const
{
  std::string filename = std::format("{}/{}-{}-{}{}", m_cache_location, zoom, x, y, m_filetype);
  return filename;
}

std::string TileService::tile_url(unsigned x, unsigned y, unsigned zoom) const
{
  std::string url;
  const unsigned num_y_tiles = (1 << zoom);

  switch (m_url_pattern) {
    case ZXY: {
      url = std::format("{}/{}/{}/{}{}", m_url, zoom, x, (num_y_tiles - y - 1), m_filetype);
      break;
    }
    case ZYX: {
      url = std::format("{}/{}/{}/{}{}", m_url, zoom, (num_y_tiles - y - 1), x, m_filetype);
      break;
    }
    case ZYX_Y_SOUTH: {
      url = std::format("{}/{}/{}/{}{}", m_url, zoom, y, x, m_filetype);
      break;
    }
    default:
      assert(false);
  }

  return url;
}

std::string TileService::download_and_save(float lat, float lon, unsigned zoom)
{
  unsigned x = wms::lon2tilex(lon, zoom);
  unsigned y = wms::lat2tiley(lat, zoom);

  std::string url = tile_url(x, y, zoom);

  std::string filename = tile_filename(x, y, zoom);

  if (std::filesystem::exists(filename)) {
    std::cout << "From disk " << std::quoted(filename) << std::endl;
    return filename;
  }

  std::ofstream of(filename, std::ios::binary);
  cpr::Response r = cpr::Download(of, cpr::Url{url});

  if (r.status_code != 200) {
    std::cerr << "Could not get tile from " << std::quoted(url) << std::endl;
    std::filesystem::remove(filename);
  } else {
    std::cout << "From web " << std::quoted(url) << std::endl;
  }

  return filename;
}

Image* TileService::get_tile(float lat, float lon, unsigned zoom)
{
  unsigned x = wms::lon2tilex(lon, zoom);
  unsigned y = wms::lat2tiley(lat, zoom);

  std::string url = tile_url(x, y, zoom);

  std::string filename = tile_filename(x, y, zoom);

  if (!std::filesystem::exists(filename)) {
    std::ofstream of(filename, std::ios::binary);
    cpr::Response r = cpr::Download(of, cpr::Url{url});

    if (r.status_code != 200) {
      std::cerr << "Could not get tile from " << std::quoted(url) << std::endl;
      std::filesystem::remove(filename);
      return nullptr;
    }
  }

  auto image = new Image();
  image->read(filename);
  if (!image->loaded()) {
    std::cerr << "Could not read " << std::quoted(filename) << std::endl;
  }

  assert(image->loaded());

  return image;
}
