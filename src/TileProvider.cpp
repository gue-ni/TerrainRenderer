#include "TileProvider.h"

#include <filesystem>


Image LocalTileProvider::get_tile(unsigned zoom, unsigned x, unsigned y)
{
  Image image;
  std::string path = std::format("{}/{}/{}/{}/texture.jpg", m_tile_root_path, zoom, x, y);
  image.read(path, true);

  if (!image.loaded()) {
    std::cerr << "Could not load " << std::quoted(path) << std::endl;
  }

  assert(image.loaded());
  return image;
}

#include <cpr/cpr.h>

WebTileProvider::WebTileProvider()
{
  if (!std::filesystem::exists(m_cache_location)) {
    std::filesystem::create_directory(m_cache_location);
  }
}

std::string WebTileProvider::download_and_save(unsigned zoom, unsigned x, unsigned y)
{
#if 1
  // albedo
  const std::string filetype = "jpeg";
  const std::string host = "https://gataki.cg.tuwien.ac.at/raw/basemap/tiles";
  const std::string url = std::format("{}/{}/{}/{}.{}", host, zoom, y, x, filetype);
#else
  // heightmap

  // convert from y pointing south to y pointing north
  unsigned num_y_tiles = (1 << zoom);
  y = num_y_tiles - y - 1;

  const std::string filetype = "png";
  const std::string host = "https://alpinemaps.cg.tuwien.ac.at/tiles/alpine_png";
  const std::string url = std::format("{}/{}/{}/{}.{}", host, zoom, x, y, filetype);
#endif

  std::string filename = std::format("{}/{}-{}-{}.{}", m_cache_location, zoom, x, y, filetype);

  if (std::filesystem::exists(filename)) {
    std::cout << "Already downloaded " << std::quoted(filename) << std::endl;
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

TileService::TileService(const std::string& url, const UrlPattern& url_pattern, const std::string& file_extension)
    : m_url(url), m_url_pattern(url_pattern), m_file_extension(file_extension)
{
}

std::string TileService::load_tile(float lat, float lon, unsigned zoom) { return std::string(); }
