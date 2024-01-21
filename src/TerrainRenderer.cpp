#include "TerrainRenderer.h"

#include <iostream>

#include "../gfx/gfx.h"

const std::string shader_vert = R"(
#version 430
layout (location = 0) in vec3 a_pos;
layout (location = 1) in vec2 a_tex;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

out vec2 uv;

void main() {
  uv = a_tex;
  gl_Position = proj * view * model * vec4(a_pos, 1.0);
}
)";

const std::string shader_frag = R"(
#version 430

in vec2 uv;

out vec4 FragColor;

uniform sampler2D u_albedo_texture;
uniform sampler2D u_heightmap_texture;

void main() {
  FragColor = vec4(texture(u_albedo_texture, uv).rgb, 1);
}
)";

constexpr uint MAX_ZOOM_LEVEL = 15;

constexpr TileName ROOT_TILE_1 = {.zoom = 11, .x = 1072, .y = 712};

constexpr TileName ROOT_TILE_2 = {
    .zoom = 9,
    .x = 277,
    .y = 179,
};

constexpr TileName ROOT_TILE_3 = {
    .zoom = 11,
    .x = 1086,
    .y = 719,
};

constexpr TileName ROOT_TILE_4 = {
    .zoom = 11,
    .x = 1101,
    .y = 1327,
};

TerrainRenderer::TerrainRenderer(const glm::vec2& min, const glm::vec2& max)
    : m_shader(std::make_unique<ShaderProgram>(shader_vert, shader_frag)),
      m_root_tile(ROOT_TILE_3),
      m_chunk(5, 1.0f),
      m_bounds({min, max}),
      m_tile_cache(min, max, m_root_tile, MAX_ZOOM_LEVEL)
{
}

void TerrainRenderer::render(const Camera& camera, const glm::vec2& center)
{
  const float min_node_size = 0.02f;
  const uint max_depth = MAX_ZOOM_LEVEL - m_root_tile.zoom;

  QuadTree quad_tree(m_bounds.min, m_bounds.max, min_node_size, max_depth);

  quad_tree.insert(center);

  auto tiles = quad_tree.get_children();

  if (wireframe) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

  m_shader->bind();
  m_shader->set_uniform("view", camera.get_view_matrix());
  m_shader->set_uniform("proj", camera.get_projection_matrix());

  for (auto* tile : tiles) {
    Texture* albedo = m_tile_cache.get_tile_texture(tile->center(), tile->depth);

    Texture* heightmap = nullptr;

    if (albedo) {
      albedo->bind(0);
      m_shader->set_uniform("u_albedo_texture", 0);
    }

    if (heightmap) {
      heightmap->bind(1);
      m_shader->set_uniform("u_heightmap_texture", 1);
    }

    m_chunk.draw(m_shader.get(), tile->min, tile->max);
  }

  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}
