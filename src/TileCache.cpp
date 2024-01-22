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
#if 1
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
  texture->set_parameter(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  texture->set_parameter(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
#if 0
  texture->set_image(image);
#else
  GLint internalformat = GL_RGB;
  GLint format = GL_RGB;

          glPixelStorei(GL_UNPACK_ALIGNMENT, 1); 

  glTexImage2D(texture->target, 0, internalformat, image.width(), image.height(), 0, format, GL_UNSIGNED_BYTE,
               image.data());
          glPixelStorei(GL_UNPACK_ALIGNMENT, 4);


#endif

  texture->generate_mipmap();
  texture->unbind();

  return texture;
}

// Texture* TileCache::load_texture_from_cache(const TileName& tile_name)
Texture* TileCache::load_texture_from_cache(float lat, float lon, unsigned zoom, const TileType& tile_type)
{
#if 0
  const size_t max_cache_size = 100;
  if (m_cache.size() > max_cache_size) {
    // TODO: only cleanup the ones not accessed recently
    m_cache.clear();
  }
#endif

  TileName tile_name = wms::to_tilename(lat, lon, zoom);
  std::string name = tile_name.to_string() + "+" + std::to_string(tile_type);

  if (!m_gpu_cache.contains(name)) {
    m_gpu_cache[name] = load_texture_from_disk(lat, lon, zoom, tile_type);
  }

  return m_gpu_cache[name].get();
}
