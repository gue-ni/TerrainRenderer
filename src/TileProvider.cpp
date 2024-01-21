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

#if 1
#include <cpr/cpr.h>

WebTileProvider::WebTileProvider()
{
  if (!std::filesystem::exists(m_cache_location)) {
    std::filesystem::create_directory(m_cache_location);
  }
}

std::string WebTileProvider::download_and_save(unsigned zoom, unsigned x, unsigned y)
{
  std::string host = "https://gataki.cg.tuwien.ac.at/raw/basemap/tiles";
  std::string url = std::format("{}/{}/{}/{}.jpeg", host, zoom, y, x);

  std::string filename = std::format("{}/{}-{}-{}.jpeg", m_cache_location, zoom, x, y);

  if (std::filesystem::exists(filename)) {
    std::cout << "Already downloaded " << std::quoted(filename) << std::endl;
    return filename;
  }

  std::ofstream of(filename, std::ios::binary);
  cpr::Response r = cpr::Download(of, cpr::Url{url});

  auto status_code = r.status_code;

  if (r.status_code != 200) {
    std::cerr << "Could not get tile from " << std::quoted(url) << std::endl;
  }

  return filename;
}
#endif
