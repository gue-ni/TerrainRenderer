#pragma once

#include <format>
#include <iostream>
#include <string>

#include "../gfx/gfx.h"

using namespace gfx;

class LocalTileProvider
{
 public:
  Image get_tile(unsigned zoom, unsigned x, unsigned y)
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

 private:
  const std::string m_tile_root_path = "C:/Users/jakob/Pictures/tiles";
};

class WebTileProvider
{
 public:
  Image get_tile(unsigned zoom, unsigned x, unsigned y)
  {
    // TODO: get tiles from remote server
    // https://gataki.cg.tuwien.ac.at/raw/basemap/tiles/17/46062/70153.jpeg
    // https://alpinemaps.cg.tuwien.ac.at/tiles/alpine_png/15/17543/21251.png
    assert(false);
    return Image();
  }

 private:
};