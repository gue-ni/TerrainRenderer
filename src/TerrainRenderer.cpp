#include "TerrainRenderer.h"

#include <iostream>

#include "../gfx/gfx.h"
#include "Common.h"

const std::string shader_vert = R"(
#version 430
layout (location = 0) in vec3 a_pos;
layout (location = 1) in vec2 a_tex;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

uniform sampler2D u_heightmap_texture;
uniform float u_height_scaling_factor;

uniform vec2 u_albedo_uv_min;
uniform vec2 u_albedo_uv_max;

out vec2 uv;



float altitude_from_color(vec4 color) {
  return (color.r + color.g / 255.0);
}

vec2 map_range(vec2 s, vec2 in_min, vec2 in_max, vec2 out_min, vec2 out_max)
{
  return out_min + (s - in_min) * (out_max - out_min) / (in_max - in_min);
}

void main() {
  uv = a_tex;
  vec4 world_pos = model * vec4(a_pos, 1.0);

#if 1

  vec2 tmp_uv = map_range(uv, vec2(0), vec2(1), u_albedo_uv_min, u_albedo_uv_max);

  vec4 height_sample = texture(u_heightmap_texture, uv);

  float height = altitude_from_color(height_sample);

  world_pos.y = height * u_height_scaling_factor;
#endif

#if 1
  if (uv.x < 0.0 || uv.x > 1.0 || uv.y < 0.0 || uv.y > 1.0) {
    world_pos.y = -10.0;
  }
#endif

  gl_Position = proj * view * world_pos;
}
)";

const std::string shader_frag = R"(
#version 430

in vec2 uv;

out vec4 FragColor;

uniform sampler2D u_albedo_texture;

void main() {
#if 1
  vec3 albedo = texture(u_albedo_texture, uv).rgb;
  FragColor = vec4(albedo, 1);
#else
  FragColor = vec4(vec3(1,0,0), 1);
#endif
}
)";

constexpr uint MAX_ZOOM_LEVEL = 11;

const TileId LUDESCH = wms::tile_id(47.1958f, 9.7793f, 8);

const TileId SCHRUNS = wms::tile_id(47.0800f, 9.9199f, 8);

const TileId GROSS_GLOCKNER = wms::tile_id(47.0742f, 12.6947f, 10);

const TileId SCHNEEBERG = wms::tile_id(47.7671f, 15.8056f, 10);

const TileId HALLSTATT = wms::tile_id(47.5622f, 13.6493f, 10);

TerrainRenderer::TerrainRenderer(const glm::vec2& min, const glm::vec2& max)
    : m_shader(std::make_unique<ShaderProgram>(shader_vert, shader_frag)),
      m_root_tile(GROSS_GLOCKNER),
      m_chunk(32, 1.0f),
      m_bounds({min, max}),
      m_tile_cache(m_root_tile, MAX_ZOOM_LEVEL)
{
  float tile_width = wms::tile_width(wms::tiley2lat(m_root_tile.y, m_root_tile.zoom), m_root_tile.zoom);

  const float min_elevation = 0.0f;
  const float max_elevation = 8191.0f;

  float width = m_bounds.size().x;

  float scaling_ratio = width / tile_width;

  m_height_scaling_factor = (max_elevation - min_elevation) * scaling_ratio;

  (void)m_tile_cache.tile_texture(m_root_tile, TileType::ORTHO);
  (void)m_tile_cache.tile_texture(m_root_tile, TileType::HEIGHT);
}

void TerrainRenderer::render(const Camera& camera, const glm::vec2& center)
{
  const float min_node_size = 0.02f;
  const uint max_depth = MAX_ZOOM_LEVEL - m_root_tile.zoom;
  const auto terrain_center = glm::clamp(center, m_bounds.min, m_bounds.max);

  QuadTree quad_tree(m_bounds.min, m_bounds.max, min_node_size, max_depth);
  quad_tree.insert(terrain_center);

  if (wireframe) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

  m_shader->bind();
  m_shader->set_uniform("view", camera.get_view_matrix());
  m_shader->set_uniform("proj", camera.get_projection_matrix());
  m_shader->set_uniform("u_height_scaling_factor", m_height_scaling_factor);

  auto render_tile = [this](Node* tile) -> bool {
    if (tile->is_leaf) {
      // check if tile exists in gpu cache
      // if yes, render it
      // if no, request it and backtrack to parent. try to render parent.

      // different approach:
      // if tile is not in cache, request it and check if parent is in cache
      // if parent is in cache, use it's texture but render only the correct
      // cutout.

      auto relative = map_to_0_1(tile->center());
      Coordinate coord = m_tile_cache.lat_lon(relative);
      TileId tile_id = m_tile_cache.tile_id(coord, tile->depth);

      Texture* albedo = m_tile_cache.tile_texture(tile_id, TileType::ORTHO);
      Texture* heightmap = m_tile_cache.tile_texture(tile_id, TileType::HEIGHT);

      glm::vec2 albedo_uv_min(0.0f), albedo_uv_max(1.0f);

#if 0
      if (!albedo) {
        // go up in tree until we find a node that is cached
        // calculate a factor for scaling the uv coords so we render it correctly

        Node* parent = tile->parent;
        if (parent) {
          TileId parent_tile_id = wms::parent_tile(tile_id);

          Texture* parent_texture = m_tile_cache.cached_tile_texture(parent_tile_id, TileType::ORTHO);

          if (parent_texture) {
            std::cout << "Found parent\n";

#if 1
            auto x = tile_id.x - (2 * parent_tile_id.x);
            auto y = tile_id.y - (2 * parent_tile_id.y);

            albedo_uv_min = glm::vec2(x * 0.5f, y * 0.5f);
            albedo_uv_max = glm::vec2((x + 1) * 0.5f, (y + 1) * 0.5f);

            albedo = parent_texture;
#else
            auto children = wms::child_tiles(parent_tile_id);

            if (tile_id == children[0]) {
              albedo_uv_min = {0.0f, 0.0f};
              albedo_uv_max = {0.5f, 0.5f};

            } else if (tile_id == children[1]) {
              albedo_uv_min = {0.5f, 0.0f};
              albedo_uv_max = {1.0f, 0.5f};

            } else if (tile_id == children[2]) {
              albedo_uv_min = {0.0f, 0.5f};
              albedo_uv_max = {0.5f, 1.0f};

            } else if (tile_id == children[3]) {
              albedo_uv_min = {0.5f, 0.5f};
              albedo_uv_max = {1.0f, 1.0f};

            } else {
              assert(false);
            }
#endif
          }
        }
      }
#endif
#if 0
      if (!albedo) {

        std::cout << "tile not found, using root\n";

        // if tile is not cached, get the root texture and scale the uv
        Texture* albedo_root = m_tile_cache.tile_texture(m_root_tile, TileType::ORTHO);
        unsigned lod_diff = m_root_tile.zoom - tile_id.zoom;
        unsigned x = (m_root_tile.x * lod_diff * 2);
        unsigned y = (m_root_tile.y * lod_diff * 2);
        unsigned num_tiles_in_root = 1 << lod_diff;
        float factor = 1.0f / num_tiles_in_root;
        albedo_uv_min = glm::vec2(x * factor, y * factor);
        albedo_uv_max = glm::vec2((x + 1) * factor, (y + 1) * factor);

        albedo = albedo_root;

      }

#endif

      if (albedo && heightmap) {
        albedo->bind(0);
        m_shader->set_uniform("u_albedo_texture", 0);
        m_shader->set_uniform("u_albedo_uv_min", albedo_uv_min);
        m_shader->set_uniform("u_albedo_uv_max", albedo_uv_max);

        heightmap->bind(1);
        m_shader->set_uniform("u_heightmap_texture", 1);
        m_shader->set_uniform("u_heightmap_uv_min", glm::vec2(0.0f));
        m_shader->set_uniform("u_heightmap_uv_max", glm::vec2(1.0f));

        m_chunk.draw(m_shader.get(), tile->min, tile->max);
      }
    }

    return true;
  };

  quad_tree.traverse(render_tile);

  // m_tile_cache.invalidate_gpu_cache();

  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

glm::vec2 TerrainRenderer::map_to_0_1(const glm::vec2& point)
{
  return map_range(point, m_bounds.min, m_bounds.max, glm::vec2(0.0f), glm::vec2(1.0f));
}
