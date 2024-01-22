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

class TileService
{
 public:
  enum UrlPattern {
    ZXY,
    ZYX,
    ZYX_Y_SOUTH,
  };

  TileService(const std::string& url, const UrlPattern& url_pattern, const std::string& file_extension = "png");

  std::string load_tile(float lat, float lon, unsigned zoom);

 private:
  const UrlPattern& m_url_pattern;
  const std::string m_url, m_file_extension;
};