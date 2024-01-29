#pragma once

#include <array>
#include <format>
#include <numbers>
#include <string>

#include "Common.h"

struct TileId {
  unsigned zoom{}, x{}, y{};

  TileId() : TileId(0, 0, 0) {}
  TileId(unsigned zoom_, unsigned x_, unsigned y_) : zoom(zoom_), x(x_), y(y_) {}
  auto operator<=>(const TileId&) const = default;
  std::string to_string() const { return std::format("{}/{}/{}", zoom, x, y); }
};

inline std::ostream& operator<<(std::ostream& os, const TileId& t)
{
  return os << t.zoom << ", " << t.x << ", " << t.y;
}

template <>
struct std::hash<TileId> {
  std::size_t operator()(TileId const& s) const noexcept
  {
#if 1
    return std::hash<std::string>{}(s.to_string());
#else
    uint64_t h1 = uint64_t(s.x) << 24;
    uint64_t h2 = uint64_t(s.y) << 8;
    uint64_t h3 = uint64_t(s.zoom);
    uint64_t h = h1 | h2 | h3;
    return std::hash<uint64_t>{}(h);
#endif
  }
};

struct Coordinate {
  float lat, lon;
  Coordinate(float lat_, float lon_) : lat(lat_), lon(lon_) {}
};

inline std::ostream& operator<<(std::ostream& os, const Coordinate& c) { return os << c.lat << ", " << c.lon; }

namespace wms
{
constexpr float PI = std::numbers::pi_v<float>;

constexpr float EARTH_RADIUS = 6378137.0f;

constexpr float EQUATORIAL_CIRCUMFERENCE = 2.0f * PI * EARTH_RADIUS;

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
  return {zoom, wms::lon2tilex(lon, zoom), wms::lat2tiley(lat, zoom)};
}

// width of tile in meters
// https://wiki.openstreetmap.org/wiki/Zoom_levels
inline float tile_width(float lat, unsigned zoom)
{
  float latrad = lat * PI / 180.0f;
  return std::abs(EQUATORIAL_CIRCUMFERENCE * std::cos(latrad) / (1 << zoom));
}

// get min and max latitude/longitude of a tile
inline Bounds<Coordinate> tile_bounds(const TileId& t)
{
  Coordinate min = {tiley2lat(t.y, t.zoom), tilex2lon(t.x, t.zoom)};
  Coordinate max = {tiley2lat(t.y + 1, t.zoom), tilex2lon(t.x + 1, t.zoom)};
  return {min, max};
}

inline TileId parent_tile(const TileId& tile) { return {tile.zoom - 1, tile.x / 2, tile.y / 2}; }

// https://wiki.openstreetmap.org/wiki/Slippy_map_tilenames#Subtiles
inline std::array<TileId, 4> child_tiles(const TileId& tile)
{
  unsigned x = tile.x, y = tile.y, child_zoom = tile.zoom + 1;
  return std::array<TileId, 4>({
      {child_zoom, 2 * x + 0, 2 * y + 0},
      {child_zoom, 2 * x + 1, 2 * y + 0},
      {child_zoom, 2 * x + 0, 2 * y + 1},
      {child_zoom, 2 * x + 1, 2 * y + 1},
  });
}

// https://en.wikipedia.org/wiki/Horizon#Derivation
inline float distance_to_horizon(float altitude) {
    return std::sqrt(2.0f * EARTH_RADIUS * altitude + (altitude * altitude));
}

// https://en.wikipedia.org/wiki/Horizon#Arc_distance
inline float geographical_distance_to_horizon(float altitude) {
    return EARTH_RADIUS * std::atan(distance_to_horizon(altitude) / EARTH_RADIUS);
}
};  // namespace wms
