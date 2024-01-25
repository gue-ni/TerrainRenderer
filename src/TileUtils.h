#pragma once

#include <format>
#include <numbers>
#include <string>

struct TileId {
  unsigned zoom, x, y;
  auto operator<=>(const TileId&) const = default;
  std::string to_string() const { return std::format("{}/{}/{}", zoom, x, y); }
};

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

// get min and max latitude/longitude of a tile
inline std::pair<Coordinate, Coordinate> tile_bounds(unsigned x, unsigned y, unsigned zoom)
{
  Coordinate min = {.lat = tiley2lat(y, zoom), .lon = tilex2lon(x, zoom)};
  Coordinate max = {.lat = tiley2lat(y + 1, zoom), .lon = tilex2lon(x + 1, zoom)};
  return {min, max};
}

inline TileId parent_tile(const TileId& tile) { return {.zoom = tile.zoom - 1, .x = tile.x / 2, .y = tile.y / 2}; }

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
