#include "TileCache.h"

#include <cassert>
#include <format>
#include <iostream>

#include "Common.h"

TileCache::TileCache(const TileName& root_tile, unsigned max_zoom_level)
    : m_root_tile(root_tile),
      m_max_zoom_level(max_zoom_level),
      m_ortho_tile_service("https://gataki.cg.tuwien.ac.at/raw/basemap/tiles", TileService::UrlPattern::ZYX_Y_SOUTH,
                           ".jpeg"),
      m_height_tile_service("https://alpinemaps.cg.tuwien.ac.at/tiles/alpine_png", TileService::UrlPattern::ZXY, ".png")

{
#if 0
  std::string path = "debug.png";
  //std::string path = "debug2.jpeg";
  //std::string path = "cache/13-4395-2868.jpeg";
  Image image;
  image.read(path);
  assert(image.loaded());

  auto filter = GL_LINEAR;
  m_debug_texture = std::make_unique<Texture>();
  m_debug_texture->bind();
  m_debug_texture->set_parameter(GL_TEXTURE_MIN_FILTER, filter);
  m_debug_texture->set_parameter(GL_TEXTURE_MAG_FILTER, filter);
  m_debug_texture->set_parameter(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  m_debug_texture->set_parameter(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  GLint internalformat = GL_RGB;
  GLint format = GL_RGB;
          glPixelStorei(GL_UNPACK_ALIGNMENT, 1); 

  glTexImage2D(m_debug_texture->target, 0, internalformat, image.width(), image.height(), 0, format, GL_UNSIGNED_BYTE,
               image.data());
          glPixelStorei(GL_UNPACK_ALIGNMENT, 4);


  m_debug_texture->generate_mipmap();
  m_debug_texture->unbind();
#else
#endif
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

  TileName tile_name;
  tile_name.zoom = zoom;
  tile_name.x = wms::lon2tilex(lon, tile_name.zoom);
  tile_name.y = wms::lat2tiley(lat, tile_name.zoom);

  return load_texture_from_cache(lat, lon, zoom, tile_type);
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
    //std::cout << "gpu cache size " << m_gpu_cache.size() << std::endl;
  }
}

std::unique_ptr<Texture> TileCache::load_texture_from_disk(float lat, float lon, unsigned zoom,
                                                           const TileType& tile_type)
{
  std::string tile_path;

  if (tile_type == TileType::ORTHO) {
    tile_path = m_ortho_tile_service.download_and_save(lat, lon, zoom);
  } else if (tile_type == TileType::HEIGHT) {
    tile_path = m_height_tile_service.download_and_save(lat, lon, zoom);
  } else {
    assert(false);
  }

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

#if 0
  texture->set_parameter(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
  texture->set_parameter(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
  const float border_color[] = {0.24f, 0.24f, 0.18f, 1.0f};
  glTexParameterfv(texture->target, GL_TEXTURE_BORDER_COLOR, border_color);
#else
  texture->set_parameter(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  texture->set_parameter(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
#endif

  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  texture->set_image(image);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

  texture->generate_mipmap();
  texture->unbind();

  return texture;
}

// Texture* TileCache::load_texture_from_cache(const TileName& tile_name)
Texture* TileCache::load_texture_from_cache(float lat, float lon, unsigned zoom, const TileType& tile_type)
{
  TileName tile_name = wms::to_tilename(lat, lon, zoom);
  std::string name = tile_name.to_string() + "+" + std::to_string(tile_type);

  if (!m_gpu_cache.contains(name)) {
    std::cout << "New in cache " << name << std::endl;
    CacheInfo info;
    info.accessed();
    auto texture = load_texture_from_disk(lat, lon, zoom, tile_type);
    m_gpu_cache[name] = std::make_tuple(info, std::move(texture));
  } else {
    //std::cout << "From gpu cache " << name << std::endl;
  }

  auto& [info, texture] = m_gpu_cache[name];
  info.accessed();  // does this create a copy or reference?
  return texture.get();
}
