#include "TileCache.h"

#include <cassert>
#include <format>
#include <iostream>

#include "Common.h"

TileCache::TileCache(const glm::vec2& min, const glm::vec2& max, const TileName& root_tile, unsigned max_zoom_level)
    : m_min(min), m_max(max), m_root_tile(root_tile), m_max_zoom_level(max_zoom_level)
{
  m_debug_texture = load_texture_from_disk(m_root_tile);
}

Texture* TileCache::get_tile_texture(const glm::vec2& point, unsigned lod)
{
  assert((m_min.x <= point.x && point.x <= m_max.x) && (m_min.y <= point.y && point.y <= m_max.y));

  unsigned zoom = m_root_tile.zoom + lod;
  assert(zoom <= m_max_zoom_level);

  Coordinate min_coord;
  min_coord.lat = wms::tiley2lat(m_root_tile.y, m_root_tile.zoom);
  min_coord.lon = wms::tilex2lon(m_root_tile.x, m_root_tile.zoom);

  Coordinate max_coord;
  max_coord.lat = wms::tiley2lat(m_root_tile.y + 1, m_root_tile.zoom);
  max_coord.lon = wms::tilex2lon(m_root_tile.x + 1, m_root_tile.zoom);

  // map to [0, 1]
  glm::vec2 factor = map_range(point, m_min, m_max, glm::vec2(0.0f), glm::vec2(1.0f));
  // factor.y = 1.0f - factor.y; // this depends on whether y points north or south

  float lat = glm::mix(min_coord.lat, max_coord.lat, factor.y);
  float lon = glm::mix(min_coord.lon, max_coord.lon, factor.x);

  TileName tile_name;
  tile_name.zoom = zoom;
  tile_name.x = wms::lon2tilex(lon, tile_name.zoom);
  tile_name.y = wms::lat2tiley(lat, tile_name.zoom);

  return load_texture_from_cache(tile_name);
}

std::unique_ptr<Texture> TileCache::load_texture_from_disk(const TileName& tile)
{
  auto tile_path = m_tile_service.download_and_save(tile.zoom, tile.x, tile.y);

  // Image image = m_provider.get_tile(tile.zoom, tile.x, tile.y);

  Image image;
  image.read(tile_path);
  assert(image.loaded());

  if (!image.loaded()) {
    return nullptr;
  }

  auto filter = GL_LINEAR;
  auto texture = std::make_unique<Texture>();
  texture->bind();
  texture->set_parameter(GL_TEXTURE_MIN_FILTER, filter);
  texture->set_parameter(GL_TEXTURE_MAG_FILTER, filter);
  texture->set_parameter(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  texture->set_parameter(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  texture->set_image(image);

  return texture;
}

Texture* TileCache::load_texture_from_cache(const TileName& tile_name)
{
  const size_t max_cache_size = 50;
  if (m_cache.size() > max_cache_size) {
    // TODO: only cleanup the ones not accessed recently
    m_cache.clear();
  }

  std::string name = tile_name.to_string();

  if (!m_cache.contains(name)) {
    m_cache[name] = load_texture_from_disk(tile_name);
    std::cout << "Cache size: " << m_cache.size() << std::endl;
  }

  return m_cache[name].get();
}
