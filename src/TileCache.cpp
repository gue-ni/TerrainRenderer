#include "TileCache.h"

#include <cassert>
#include <format>
#include <iostream>

#include "Common.h"

TileCache::TileCache(const TileId& root_tile, unsigned max_zoom_level)
    : m_root_tile(root_tile),
      m_max_zoom_level(max_zoom_level),
      m_min_coord(wms::tiley2lat(root_tile.y + 0, root_tile.zoom), wms::tilex2lon(m_root_tile.x + 0, m_root_tile.zoom)),
      m_max_coord(wms::tiley2lat(root_tile.y + 1, root_tile.zoom), wms::tilex2lon(m_root_tile.x + 1, m_root_tile.zoom)),
      m_ortho_service("https://gataki.cg.tuwien.ac.at/raw/basemap/tiles", UrlPattern::ZYX_Y_SOUTH, ".jpeg"),
      m_height_service("https://alpinemaps.cg.tuwien.ac.at/tiles/alpine_png", UrlPattern::ZXY_Y_NORTH, ".png")

{
  m_ortho_service.start_worker_threads();
  m_height_service.start_worker_threads();
}

Texture* TileCache::tile_texture(const TileId& tile, const TileType& tile_type)
{
  std::string name = tile.to_string() + "+" + std::to_string(tile_type);

  if (m_gpu_cache.contains(name)) {
    return m_gpu_cache[name].get();
  }

  Image* image = request_image(tile, tile_type);

  if (image) {
    auto texture = create_texture(*image);
    m_gpu_cache[name] = std::move(texture);
    return m_gpu_cache[name].get();
  }

  return nullptr;
}

Texture* TileCache::tile_texture_sync(const TileId& tile, const TileType& tile_type)
{
  std::string name = tile.to_string() + "+" + std::to_string(tile_type);

  Image* image = nullptr;

  if (m_gpu_cache.contains(name)) {
    return m_gpu_cache[name].get();
  } else {
    switch (tile_type) {
      case TileType::ORTHO:
        image = m_ortho_service.get_tile_sync(tile);
        break;
      case TileType::HEIGHT:
        image = m_height_service.get_tile_sync(tile);
        break;
      default:
        assert(false);
    }

    auto texture = create_texture(*image);
    m_gpu_cache[name] = std::move(texture);
    return m_gpu_cache[name].get();
  }

  return nullptr;
}

Texture* TileCache::tile_texture_cached(const TileId& tile, const TileType& tile_type)
{
  std::string name = tile.to_string() + "+" + std::to_string(tile_type);
  return m_gpu_cache[name].get();
}

void TileCache::invalidate_gpu_cache()
{
#if 0
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
#endif
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

Image* TileCache::request_image(const TileId& tile, const TileType& tile_type)
{
  switch (tile_type) {
    case TileType::ORTHO:
      return m_ortho_service.get_tile(tile);

    case TileType::HEIGHT:
      return m_height_service.get_tile(tile);

    default:
      assert(false);
      return nullptr;
  }
}

Coordinate TileCache::lat_lon(const glm::vec2& point) const
{
  // point is in range [0, 1]
  assert(glm::all(glm::lessThanEqual(glm::vec2(0.0f), point)) && glm::all(glm::lessThanEqual(point, glm::vec2(1.0f))));
  float lat = glm::mix(m_min_coord.lat, m_max_coord.lat, point.y);
  float lon = glm::mix(m_min_coord.lon, m_max_coord.lon, point.x);
  return {lat, lon};
}

TileId TileCache::tile_id(Coordinate& coord, unsigned lod_offset_from_root) const
{
  return wms::tile_id(coord.lat, coord.lon, m_root_tile.zoom + lod_offset_from_root);
}

float TileCache::terrain_elevation(const Coordinate& point) const { return 0; }
