#pragma once

#include <format>
#include <iostream>
#include <string>

#include "../gfx/gfx.h"

using namespace gfx;

class LocalTileProvider
{
 public:
  Image get_tile(unsigned zoom, unsigned x, unsigned y);

 private:
  const std::string m_tile_root_path = "C:/Users/jakob/Pictures/tiles";
};

class WebTileProvider
{
 public:
  WebTileProvider();
  std::string download_and_save(unsigned zoom, unsigned x, unsigned y);

 private:
  const std::string m_cache_location{"cache"};
};