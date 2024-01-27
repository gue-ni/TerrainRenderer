#pragma once

#include <format>
#include <numbers>
#include <string>

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
    std::size_t h1 = std::hash<unsigned>{}(s.x);
    std::size_t h2 = std::hash<unsigned>{}(s.y);
    std::size_t h3 = std::hash<unsigned>{}(s.zoom);
    return h1 ^ h2 ^ h3;  // TODO: Is this a good idea?
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
  const float C = 40075016.686f;
  return std::abs(C * std::cos(lat) / (1 << zoom));
}

// get min and max latitude/longitude of a tile
inline std::pair<Coordinate, Coordinate> tile_bounds(unsigned x, unsigned y, unsigned zoom)
{
  Coordinate min = {tiley2lat(y, zoom), tilex2lon(x, zoom)};
  Coordinate max = {tiley2lat(y + 1, zoom), tilex2lon(x + 1, zoom)};
  return {min, max};
}

inline TileId parent_tile(const TileId& tile) { return {tile.zoom - 1, tile.x / 2, tile.y / 2}; }

// https://wiki.openstreetmap.org/wiki/Slippy_map_tilenames#Subtiles
inline std::array<TileId, 4> child_tiles(const TileId& tile)
{
  unsigned x = tile.x, y = tile.y, new_zoom = tile.zoom + 1;
  return std::array<TileId, 4>({
      {new_zoom, 2 * x + 0, 2 * y + 0},
      {new_zoom, 2 * x + 1, 2 * y + 0},
      {new_zoom, 2 * x + 0, 2 * y + 1},
      {new_zoom, 2 * x + 1, 2 * y + 1},
  });
}

};  // namespace wms
