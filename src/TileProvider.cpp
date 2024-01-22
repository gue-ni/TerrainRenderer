#include "TileProvider.h"

#include <cpr/cpr.h>

#include <filesystem>

#include "TileCache.h"

TileService::TileService(const std::string& url, const UrlPattern& url_pattern, const std::string& filetype)
    : m_url(url), m_url_pattern(url_pattern), m_filetype(filetype)
{
  if (!std::filesystem::exists(m_cache_location)) {
    std::filesystem::create_directory(m_cache_location);
  }
}

std::string TileService::download_and_save(float lat, float lon, unsigned zoom)
{
  unsigned x = wms::lon2tilex(lon, zoom);
  unsigned y = wms::lat2tiley(lat, zoom);
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

  std::string filename = std::format("{}/{}-{}-{}{}", m_cache_location, zoom, x, y, m_filetype);

  if (std::filesystem::exists(filename)) {
    //std::cout << "Already downloaded " << std::quoted(filename) << std::endl;
    return filename;
  }

  std::ofstream of(filename, std::ios::binary);
  cpr::Response r = cpr::Download(of, cpr::Url{url});

  if (r.status_code != 200) {
    std::cerr << "Could not get tile from " << std::quoted(url) << std::endl;
    std::filesystem::remove(filename);
  }

  return filename;
}

