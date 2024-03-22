#include "TileCache.h"

#include <cassert>
#include <iostream>

#include "Common.h"

TileCache::TileCache()
    :
#if 0
      m_ortho_service("https://gataki.cg.tuwien.ac.at/raw/basemap/tiles", UrlPattern::ZYX_Y_SOUTH, ".jpeg", "tiles/ortho-1"),
#else
      m_ortho_service("https://server.arcgisonline.com/ArcGIS/rest/services/World_Imagery/MapServer/tile",
                      UrlPattern::ZYX_Y_SOUTH, "", "tiles/ortho-2"),
#endif
      m_height_service("https://www.jakobmaier.at/tiles/dem", UrlPattern::ZXY_Y_NORTH, ".png", "tiles/height-1")
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

float TileCache::elevation(const Coordinate& coord)
{
  TileId tile(coord, 7);  // probably not the best, as this is very low res

  Bounds<Coordinate> bounds = tile.bounds();

  Image* image = m_height_service.get_tile(tile);
  if (!image) {
    image = m_height_service.get_tile_sync(tile);
  }

  assert(image);

  auto val = coord.to_vec2();
  auto min = bounds.min.to_vec2();
  auto max = bounds.max.to_vec2();

  glm::vec2 uv = map_range(val, min, max, glm::vec2(0.0f), glm::vec2(1.0f));

  glm::u8vec4 pixel = image->sample(uv);

  glm::vec3 sample = gfx::rgb(pixel.r, pixel.g, pixel.b);

  return sample.r + (sample.b / 255.0f);
}
