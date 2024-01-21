#include "TileCache.h"

#include <cassert>
#include <format>
#include <iostream>

TileCache::TileCache(const glm::vec2& min, const glm::vec2& max) : m_min(min), m_max(max)
{
  m_debug_texture = load_texture(m_root_tile);
}

Texture* TileCache::get_tile_texture(const glm::vec2& min, const glm::vec2& max, unsigned lod)
{
  // assert((m_min <= min && min <= m_max) && (min <= m_max && m_max <= max));
  // TODO: calculate which tile to get from disk
  return m_debug_texture.get();
}

std::unique_ptr<Texture> TileCache::load_texture(const TileName& tile_name)
{
  std::string path = std::format("{}/{}/{}/{}/texture.jpg", m_tile_root_path, tile_name.zoom, tile_name.x, tile_name.y);

  Image image;
  image.read(path);
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
