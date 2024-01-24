#include "TileCache.h"

#include <cassert>
#include <format>
#include <iostream>

#include "Common.h"

TileCache::TileCache(const TileId& root_tile, unsigned max_zoom_level)
    : m_root_tile(root_tile),
      m_max_zoom_level(max_zoom_level),
      m_ortho_tile_service("https://gataki.cg.tuwien.ac.at/raw/basemap/tiles", TileService::UrlPattern::ZYX_Y_SOUTH,
                           ".jpeg"),
      m_height_tile_service("https://alpinemaps.cg.tuwien.ac.at/tiles/alpine_png", TileService::UrlPattern::ZXY, ".png")

{
}

Texture* TileCache::get_tile_texture(const glm::vec2& point, unsigned lod, const TileType& tile_type)
{
  unsigned zoom = m_root_tile.zoom + lod;
  assert(zoom <= m_max_zoom_level);

  // point is in range [0, 1]
  assert(glm::all(glm::lessThanEqual(glm::vec2(0.0f), point)) && glm::all(glm::lessThanEqual(point, glm::vec2(1.0f))));

  Coordinate min_coord;
  min_coord.lat = wms::tiley2lat(m_root_tile.y, m_root_tile.zoom);
  min_coord.lon = wms::tilex2lon(m_root_tile.x, m_root_tile.zoom);

  Coordinate max_coord;
  max_coord.lat = wms::tiley2lat(m_root_tile.y + 1, m_root_tile.zoom);
  max_coord.lon = wms::tilex2lon(m_root_tile.x + 1, m_root_tile.zoom);

  float lat = glm::mix(min_coord.lat, max_coord.lat, point.y);
  float lon = glm::mix(min_coord.lon, max_coord.lon, point.x);

  return load_texture(lat, lon, zoom, tile_type);
}

void TileCache::invalidate_gpu_cache()
{
  auto now = std::chrono::system_clock::now();
  const std::chrono::milliseconds max_duration(2000);  // TODO: find sensible value

  uint removed = 0;

  for (auto it = m_gpu_cache.begin(); it != m_gpu_cache.end();) {
    auto& [cache_info, texture] = it->second;
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - cache_info.last_accessed);

    if (duration > max_duration) {
      it = m_gpu_cache.erase(it);  // does this really delete the unique_ptr?
      removed++;
    } else {
      ++it;
    }
  }

  if (removed > 0) {
    // std::cout << "gpu cache size " << m_gpu_cache.size() << std::endl;
  }
}

Texture* TileCache::load_texture(float lat, float lon, unsigned zoom, const TileType& tile_type)
{
  TileId tile_name = wms::tile_id(lat, lon, zoom);
  std::string name = tile_name.to_string() + "+" + std::to_string(tile_type);

  if (m_gpu_cache.contains(name)) {
    auto& [info, texture] = m_gpu_cache[name];
    info.accessed();
    return texture.get();
  } else {
    Image* image = request_image(lat, lon, zoom, tile_type);

    if (image) {
      CacheInfo info;
      info.accessed();
      auto texture = create_texture(*image);
      auto texture_ptr = texture.get();
      m_gpu_cache[name] = {info, std::move(texture)};
      return texture_ptr;
    } else {
      return nullptr;
    }
  }
}

std::unique_ptr<Texture> TileCache::create_texture(const Image& image)
{
  auto texture = std::make_unique<Texture>();
  texture->bind();
  texture->set_parameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  texture->set_parameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  texture->set_parameter(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  texture->set_parameter(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  texture->set_image(image);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

  texture->generate_mipmap();
  texture->unbind();
  return texture;
}

Image* TileCache::request_image(float lat, float lon, unsigned zoom, const TileType& tile_type)
{
  switch (tile_type) {
    case TileType::ORTHO:
      return m_ortho_tile_service.get_tile(lat, lon, zoom);

    case TileType::HEIGHT:
      return m_height_tile_service.get_tile(lat, lon, zoom);

    default:
      assert(false);
      return nullptr;
  }
}
