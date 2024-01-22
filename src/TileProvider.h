#pragma once

#include <format>
#include <iostream>
#include <string>

#include "../gfx/gfx.h"

using namespace gfx;

class TileService
{
 public:
  enum UrlPattern {
    ZXY,
    ZYX_Y_SOUTH,
  };

  TileService(const std::string& url, const UrlPattern& url_pattern, const std::string& filetype = "png");

  std::string download_and_save(float lat, float lon, unsigned zoom);

 private:
  const UrlPattern m_url_pattern;
  const std::string m_url, m_filetype;
  const std::string m_cache_location{"cache"};
};