#include "TileCache.h"

#include <cassert>
#include <format>
#include <iostream>

#include "Common.h"

TileCache::TileCache(const glm::vec2& min, const glm::vec2& max)
    : m_min(min), m_max(max), m_root_tile({.zoom = 11, .x = 1072, .y = 712}), m_max_zoom_level(14)
{
  // m_debug_texture = load_texture_from_disk(m_root_tile);
}

TileCache::TileCache(const glm::vec2& min, const glm::vec2& max, const TileName& root_tile, unsigned max_zoom_level)
    : m_min(min), m_max(max), m_root_tile(root_tile), m_max_zoom_level(max_zoom_level)
{
}

Texture* TileCache::get_tile_texture(const glm::vec2& point, unsigned lod)
{
  assert((m_min.x <= point.x && point.x <= m_max.x) && (m_min.y <= point.y && point.y <= m_max.y));

  unsigned zoom = m_root_tile.zoom + lod;
  assert(zoom <= m_max_zoom_level);

  Coordinate min_coord;
  min_coord.lat = wms::tiley2lat(m_root_tile.y, m_root_tile.zoom);
  min_coord.lon = wms::tilex2long(m_root_tile.x, m_root_tile.zoom);

  Coordinate max_coord;
  max_coord.lat = wms::tiley2lat(m_root_tile.y + 1, m_root_tile.zoom);
  max_coord.lon = wms::tilex2long(m_root_tile.x + 1, m_root_tile.zoom);

  // map to [0, 1]
  glm::vec2 factor = map_range(point, m_min, m_max, glm::vec2(0.0f), glm::vec2(1.0f));
  factor.y = 1.0f - factor.y;

  float lat = glm::mix(min_coord.lat, max_coord.lat, factor.y);
  float lon = glm::mix(min_coord.lon, max_coord.lon, factor.x);

  TileName tile_name;
  tile_name.zoom = zoom;
  tile_name.x = wms::long2tilex(lon, tile_name.zoom);
  tile_name.y = wms::lat2tiley(lat, tile_name.zoom);

  return load_texture_from_cache(tile_name);
}

std::unique_ptr<Texture> TileCache::load_texture_from_disk(const TileName& tile)
{
  std::cout << "load from disk: " << tile.to_string() << std::endl;

  std::string path = std::format("{}/{}/{}/{}/texture.jpg", m_tile_root_path, tile.zoom, tile.x, tile.y);

  Image image;
  image.read(path, true);

  if (!image.loaded()) {
    std::cerr << "Could not load " << std::quoted(path) << std::endl;
  }

  assert(image.loaded());

  auto texture = std::make_unique<Texture>();
  texture->bind();
  texture->set_parameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  texture->set_parameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  texture->set_parameter(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  texture->set_parameter(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  texture->set_image(image);

  return texture;
}

Texture* TileCache::load_texture_from_cache(const TileName& tile_name)
{
  std::string name = tile_name.to_string();

  if (!m_cache.contains(name)) {
    m_cache[name] = load_texture_from_disk(tile_name);
    std::cout << "Cache size: " << m_cache.size() << std::endl;
  }
  return m_cache[name].get();
}
