#include "TileCache.h"

#include <cassert>
#include <fmt/core.h>
#include <iostream>

#include "../gfx/util.h"
#include "Common.h"

TileCache::TileCache(const TileId& root_tile)
    : m_root_tile(root_tile),
#if 0
      m_ortho_service("https://gataki.cg.tuwien.ac.at/raw/basemap/tiles", UrlPattern::ZYX_Y_SOUTH, ".jpeg", "tiles/ortho_1"),
#else
      m_ortho_service("https://server.arcgisonline.com/ArcGIS/rest/services/World_Imagery/MapServer/tile",
                      UrlPattern::ZYX_Y_SOUTH, "", "tiles/ortho_2"),
#endif
      m_height_service("https://alpinemaps.cg.tuwien.ac.at/tiles/alpine_png", UrlPattern::ZXY_Y_NORTH, ".png",
                       "tiles/height")

{
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
  }

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

  assert(image);
  if (!image) return nullptr;

  auto texture = create_texture(*image);
  m_gpu_cache[name] = std::move(texture);
  return m_gpu_cache[name].get();
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

float TileCache::terrain_elevation(const Coordinate& coord)
{
  TileId tile(coord, m_root_tile.zoom + 1);  // probably not the best, as this is very low res

  Bounds<Coordinate> bounds = tile.bounds();

  Image* image = m_height_service.get_tile(tile);
  if (!image) return 0.0f;

  assert(image);

  auto val = coord.to_vec2();
  auto min = bounds.min.to_vec2();
  auto max = bounds.max.to_vec2();

  glm::vec2 uv = map_range(val, min, max, glm::vec2(0.0f), glm::vec2(1.0f));

  glm::u8vec4 pixel = image->sample(uv);

  glm::vec3 sample = gfx::rgb(pixel.r, pixel.g, pixel.b);

  return sample.r + (sample.b / 255.0f);
}
