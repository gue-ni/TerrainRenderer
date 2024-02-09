#pragma once

#include <array>
#include <format>
#include <numbers>
#include <string>

#include "Common.h"

// https://wiki.openstreetmap.org/wiki/Slippy_map_tilenames
namespace wms
{
constexpr float PI = std::numbers::pi_v<float>;

constexpr float EARTH_RADIUS = 6378137.0f;

constexpr float EQUATORIAL_CIRCUMFERENCE = 2.0f * PI * EARTH_RADIUS;

inline unsigned lon2tilex(float lon, unsigned zoom)
{
  return (unsigned)(std::floor((lon + 180.0f) / 360.0f * (1 << zoom)));
}

inline unsigned lat2tiley(float lat, unsigned zoom)
{
  float latrad = lat * PI / 180.0f;
  return (unsigned)(std::floor((1.0f - std::asinh(std::tan(latrad)) / PI) / 2.0f * (1 << zoom)));
}

inline float tilex2lon(unsigned x, unsigned zoom) { return x / (float)(1 << zoom) * 360.0f - 180.0f; }

inline float tiley2lat(unsigned y, unsigned zoom)
{
  float n = PI - 2.0f * PI * y / (float)(1 << zoom);
  return 180.0f / PI * std::atan(0.5f * (std::exp(n) - std::exp(-n)));
}

// width of tile in meters
// https://wiki.openstreetmap.org/wiki/Zoom_levels
inline float tile_width(float lat, unsigned zoom)
{
  float latrad = lat * PI / 180.0f;
  return std::abs(EQUATORIAL_CIRCUMFERENCE * std::cos(latrad) / (1 << zoom));
}

// https://en.wikipedia.org/wiki/Horizon#Derivation
inline float distance_to_horizon(float altitude)
{
  return std::sqrt(2.0f * EARTH_RADIUS * altitude + (altitude * altitude));
}

// https://en.wikipedia.org/wiki/Horizon#Arc_distance
inline float geographical_distance_to_horizon(float altitude)
{
  return EARTH_RADIUS * std::atan(distance_to_horizon(altitude) / EARTH_RADIUS);
}
};  // namespace wms

struct Coordinate {
  float lat, lon;
  Coordinate(float lat_, float lon_) : lat(lat_), lon(lon_) {}
  Coordinate(const glm::vec2& lat_lon) : Coordinate(lat_lon.y, lat_lon.x) {}
  glm::vec2 to_vec2() const { return {lon, lat}; }
};

// y-axis points south, so y=0 is the northern most tile.
struct TileId {
  unsigned zoom, x, y;
  static const unsigned MAX_ZOOM{16U};
  static const unsigned MAX_X{1 << MAX_ZOOM};
  static const unsigned MAX_Y{1 << MAX_ZOOM};

  TileId() : TileId(0U, 0U, 0U) {}

  explicit TileId(unsigned zoom_, unsigned x_, unsigned y_) : zoom(zoom_), x(x_), y(y_) {}

  explicit TileId(float lat, float lon, unsigned zoom_)
      : TileId(zoom_, wms::lon2tilex(lon, zoom_), wms::lat2tiley(lat, zoom_))
  {
  }

  explicit TileId(const Coordinate& coord, unsigned zoom_) : TileId(coord.lat, coord.lon, zoom_) {}

  auto operator<=>(const TileId&) const = default;

  inline std::string to_string() const { return std::format("{}-{}-{}", zoom, x, y); }

  inline Bounds<Coordinate> bounds() const
  {
    Coordinate min(wms::tiley2lat(y + 0U, zoom), wms::tilex2lon(x + 0U, zoom));
    Coordinate max(wms::tiley2lat(y + 1U, zoom), wms::tilex2lon(x + 1U, zoom));
    return {min, max};
  }

  inline TileId parent() const { return TileId(zoom - 1U, x / 2U, y / 2U); }

  inline std::array<TileId, 4> children() const
  {
    unsigned child_zoom = zoom + 1U;
    return std::array<TileId, 4>({
        TileId(child_zoom, 2U * x + 0U, 2U * y + 0U),
        TileId(child_zoom, 2U * x + 1U, 2U * y + 0U),
        TileId(child_zoom, 2U * x + 0U, 2U * y + 1U),
        TileId(child_zoom, 2U * x + 1U, 2U * y + 1U),
    });
  }
};

template <>
struct std::hash<TileId> {
  std::size_t operator()(const TileId& t) const noexcept { return std::hash<std::string>{}(t.to_string()); }
};

inline std::ostream& operator<<(std::ostream& os, const Coordinate& c) { return os << c.lat << ", " << c.lon; }

inline std::ostream& operator<<(std::ostream& os, const TileId& t) { return os << t.to_string(); }
