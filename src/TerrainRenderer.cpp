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

void main() {
  //FragColor = vec4(uv, 0, 1);
  FragColor = vec4(texture(u_albedo_texture, uv).rgb, 1);
}
)";

constexpr uint MAX_ZOOM_LEVEL = 14;
constexpr TileName ROOT_TILE = {.zoom = 11, .x = 1072, .y = 712};

TerrainRenderer::TerrainRenderer(const glm::vec2& min, const glm::vec2& max)
    : m_shader(std::make_unique<ShaderProgram>(shader_vert, shader_frag)),
      m_debug_chunk(5, 1.0f),
      m_bounds({min, max}),
      m_tile_cache(min, max, ROOT_TILE, MAX_ZOOM_LEVEL)
{
}

void TerrainRenderer::render(const Camera& camera, const glm::vec2& center)
{
  const float min_node_size = 0.02f;
  const uint max_depth = 3;

  QuadTree quad_tree(m_bounds.min, m_bounds.max, min_node_size, max_depth);

  quad_tree.insert(center);

  auto tiles = quad_tree.get_children();

  if (wireframe) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

  m_shader->bind();
  m_shader->set_uniform("view", camera.get_view_matrix());
  m_shader->set_uniform("proj", camera.get_projection_matrix());

#if 1
  for (auto* tile : tiles) {
    Texture* albedo = m_tile_cache.get_tile_texture(tile->center(), tile->depth);

    if (albedo) {
      albedo->bind(0);
      m_shader->set_uniform("u_albedo_texture", 0);
    }
    m_debug_chunk.draw(m_shader.get(), tile->min, tile->max);
  }
#endif

#if 0
  m_debug_chunk.draw(m_shader.get(), center);
#endif

  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}
