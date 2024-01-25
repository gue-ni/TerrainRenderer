#pragma once

#include <format>
#include <numbers>
#include <string>

struct TileId {
  unsigned zoom, x, y;
  std::string to_string() const { return std::format("{}/{}/{}", zoom, x, y); }
};

struct Coordinate {
  float lat, lon;
};

namespace wms
{
constexpr float PI = std::numbers::pi_v<float>;

inline unsigned num_tiles(unsigned zoom) { return (1 << zoom); }

inline unsigned lon2tilex(float lon, unsigned z) { return (int)(floor((lon + 180.0f) / 360.0f * (1 << z))); }

inline unsigned lat2tiley(float lat, unsigned z)
{
  float latrad = lat * PI / 180.0f;
  return (unsigned)(floor((1.0f - asinh(tan(latrad)) / PI) / 2.0f * (1 << z)));
}

inline float tilex2lon(unsigned x, unsigned z) { return x / (float)(1 << z) * 360.0f - 180.0f; }

inline float tiley2lat(unsigned y, unsigned z)
{
  float n = PI - 2.0f * PI * y / (float)(1 << z);
  return 180.0f / PI * atan(0.5f * (exp(n) - exp(-n)));
}

inline TileId tile_id(float lat, float lon, unsigned zoom)
{
  unsigned x = wms::lon2tilex(lon, zoom);
  unsigned y = wms::lat2tiley(lat, zoom);
  return {.zoom = zoom, .x = x, .y = y};
}

// width of tile in meters
inline float tile_width(float lat, unsigned zoom)
{
  const float C = 40075016.686f;
  return std::abs(C * std::cos(lat) / (1 << zoom));
}

inline std::pair<Coordinate, Coordinate> tile_bounds(unsigned x, unsigned y, unsigned zoom)
{
  Coordinate min = {.lat = tiley2lat(y, zoom), .lon = tilex2lon(x, zoom)};
  Coordinate max = {.lat = tiley2lat(y + 1, zoom), .lon = tilex2lon(x + 1, zoom)};
  return {min, max};
}

};  // namespace wms
