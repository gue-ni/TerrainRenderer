#include "TerrainRenderer.h"

#include <cassert>
#include <chrono>
#include <iostream>
#include <thread>

#include "../gfx/gfx.h"
#include "Common.h"

const std::string shader_vert = R"(
#version 430
layout (location = 0) in vec3 a_pos;
layout (location = 1) in vec2 a_tex;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

uniform float u_height_scaling_factor;

uniform vec2 u_height_uv_min;
uniform vec2 u_height_uv_max;
uniform sampler2D u_height_texture;

out vec2 uv;

float altitude_from_color(vec4 color) { 
  return (color.r + color.g / 255.0); 
}

vec2 map_range(vec2 s, vec2 in_min, vec2 in_max, vec2 out_min, vec2 out_max) { 
  return out_min + (s - in_min) * (out_max - out_min) / (in_max - in_min); 
}

void main() {
  uv = a_tex;

  vec4 world_pos = model * vec4(a_pos, 1.0);

#if 1

  vec2 scaled_uv = map_range(uv, vec2(0), vec2(1), u_height_uv_min, u_height_uv_max);

  vec4 height_sample = texture(u_height_texture, scaled_uv);

  float height = altitude_from_color(height_sample);

  world_pos.y = height * u_height_scaling_factor;

#endif

  // skirts on tiles
  if (uv.x < 0.0 || uv.x > 1.0 || uv.y < 0.0 || uv.y > 1.0) {
    world_pos.y = -10.0;
  }

  gl_Position = proj * view * world_pos;
}
)";

const std::string shader_frag = R"(
#version 430

in vec2 uv;

out vec4 FragColor;

uniform vec2 u_albedo_uv_min;
uniform vec2 u_albedo_uv_max;
uniform sampler2D u_albedo_texture;

vec2 map_range(vec2 s, vec2 in_min, vec2 in_max, vec2 out_min, vec2 out_max) { 
  return out_min + (s - in_min) * (out_max - out_min) / (in_max - in_min); 
}

void main() {
  vec2 scaled_uv = map_range(uv, vec2(0.0), vec2(1.0), u_albedo_uv_min, u_albedo_uv_max);

  vec3 albedo = texture(u_albedo_texture, scaled_uv).rgb;

  FragColor = vec4(albedo, 1);
}
)";

TerrainRenderer::TerrainRenderer(const TileId& root_tile, unsigned zoom_levels, const Bounds<glm::vec2>& bounds)
    : m_shader(std::make_unique<ShaderProgram>(shader_vert, shader_frag)),
      m_root_tile(root_tile),
      m_chunk(32, 1.0f),
      m_bounds(bounds),
      m_tile_cache(m_root_tile, m_root_tile.zoom + zoom_levels),
      m_zoom_levels(zoom_levels)
{
  const float min_elevation = 0.0f, max_elevation = 8191.0f;

  float width = m_bounds.size().x;

  float tile_width = wms::tile_width(wms::tiley2lat(m_root_tile.y, m_root_tile.zoom), m_root_tile.zoom);

  float scaling_ratio = width / tile_width;

  m_height_scaling_factor = (max_elevation - min_elevation) * scaling_ratio;

  (void)m_tile_cache.tile_texture_sync(m_root_tile, TileType::ORTHO);
  (void)m_tile_cache.tile_texture_sync(m_root_tile, TileType::HEIGHT);
}

void TerrainRenderer::render(const Camera& camera, const glm::vec2& center)
{
  const auto terrain_center = glm::clamp(center, m_bounds.min, m_bounds.max);

  QuadTree quad_tree(m_bounds.min, m_bounds.max, m_zoom_levels);
  quad_tree.insert(terrain_center);

  if (wireframe) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

  m_shader->bind();
  m_shader->set_uniform("view", camera.get_view_matrix());
  m_shader->set_uniform("proj", camera.get_projection_matrix());
  m_shader->set_uniform("u_height_scaling_factor", m_height_scaling_factor * 2.0f);

  auto render_tile = [this](Node* tile) -> bool {
    if (!tile->is_leaf) return true;

    TileId tile_id = tile_id_from_node(tile);

    Texture* albedo = m_tile_cache.tile_texture(tile_id, TileType::ORTHO);
    Texture* heightmap = m_tile_cache.tile_texture(tile_id, TileType::HEIGHT);

    glm::vec2 albedo_uv_min(0.0f), albedo_uv_max(1.0f);
    glm::vec2 height_uv_min(0.0f), height_uv_max(1.0f);

    if (!albedo) {
      Texture* parent_texture = nullptr;

      Node* parent = tile->parent;
      TileId parent_tile_id;

      while (parent != nullptr) {
        parent_tile_id = tile_id_from_node(parent);
        parent_texture = m_tile_cache.cached_tile_texture(parent_tile_id, TileType::ORTHO);

        if (parent_texture) {
          break;
        } else {
          parent = parent->parent;
        }
      }

      unsigned zoom_delta = tile_id.zoom - parent_tile_id.zoom;
      unsigned num_tiles = 1 << zoom_delta;
      TileId scaled_root_tile = {tile_id.zoom, parent_tile_id.x * num_tiles, parent_tile_id.y * num_tiles};
      unsigned delta_x = tile_id.x - scaled_root_tile.x, delta_y = tile_id.y - scaled_root_tile.y;
      float factor = 1.0f / num_tiles;

      albedo_uv_min = glm::vec2((delta_x + 0) * factor, (delta_y + 0) * factor);
      albedo_uv_max = glm::vec2((delta_x + 1) * factor, (delta_y + 1) * factor);
      albedo = parent_texture;
    }

    if (!heightmap) {
      Texture* heightmap_root = m_tile_cache.cached_tile_texture(m_root_tile, TileType::HEIGHT);
      assert(heightmap_root);

      unsigned zoom_delta = tile_id.zoom - m_root_tile.zoom;
      unsigned num_tiles = 1 << zoom_delta;
      TileId scaled_root_tile = {tile_id.zoom, m_root_tile.x * num_tiles, m_root_tile.y * num_tiles};
      unsigned delta_x = tile_id.x - scaled_root_tile.x, delta_y = tile_id.y - scaled_root_tile.y;
      float factor = 1.0f / num_tiles;

      height_uv_min = glm::vec2((delta_x + 0) * factor, (delta_y + 0) * factor);
      height_uv_max = glm::vec2((delta_x + 1) * factor, (delta_y + 1) * factor);

      heightmap = heightmap_root;
    }

    if (albedo && heightmap) {
      albedo->bind(0);
      m_shader->set_uniform("u_albedo_texture", 0);
      m_shader->set_uniform("u_albedo_uv_min", albedo_uv_min);
      m_shader->set_uniform("u_albedo_uv_max", albedo_uv_max);

      heightmap->bind(1);
      m_shader->set_uniform("u_height_texture", 1);
      m_shader->set_uniform("u_height_uv_min", height_uv_min);
      m_shader->set_uniform("u_height_uv_max", height_uv_max);

      m_chunk.draw(m_shader.get(), tile->min, tile->max);
    }

    return true;
  };

  auto children = quad_tree.children();
  std::sort(children.begin(), children.end(), [](Node* a, Node* b) { return a->depth > b->depth; });
  std::for_each(children.begin(), children.end(), render_tile);

  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

glm::vec2 TerrainRenderer::map_to_0_1(const glm::vec2& point)
{
  return map_range(point, m_bounds.min, m_bounds.max, glm::vec2(0.0f), glm::vec2(1.0f));
}

TileId TerrainRenderer::tile_id_from_node(Node* node)
{
  auto relative = map_to_0_1(node->center());
  Coordinate coord = m_tile_cache.lat_lon(relative);
  TileId tile_id = m_tile_cache.tile_id(coord, node->depth);
  return tile_id;
}
