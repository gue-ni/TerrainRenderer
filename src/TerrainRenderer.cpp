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
#if 0
  gl_Position = vec4(a_pos, 1);
#else
  gl_Position = proj * view * model * vec4(a_pos, 1.0);
#endif
}
)";

const std::string shader_frag = R"(
#version 430
out vec4 FragColor;
in vec2 uv;
uniform sampler2D u_heightmap;
void main() {
  //FragColor = vec4(uv, 0, 1);
  FragColor = vec4(1,0,0,1);
}
)";

TerrainRenderer::TerrainRenderer()
    : m_shader(std::make_unique<ShaderProgram>(shader_vert, shader_frag)),
      m_debug_chunk(5, 1.0f),
      m_bounds({glm::vec2(-1.0f), glm::vec2(1.0f)})
{
}

void TerrainRenderer::render(const Camera& camera, const glm::vec2& center)
{
  // TODO:
  // construct QuadTree
  // if changes are necessary, load new tiles
  // render tiles

  const float min_node_size = 0.05f;
  QuadTree quad_tree(m_bounds.min, m_bounds.max, min_node_size);

  quad_tree.insert(center);

  auto tiles = quad_tree.get_children();

  //std::cout << "Tile Count: " << tiles.size() << std::endl;

  if (m_wireframe) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

  m_shader->bind();
  m_shader->set_uniform("view", camera.get_view_matrix());
  m_shader->set_uniform("proj", camera.get_projection_matrix());

#if 1
  for (auto* tile : tiles) {
    m_debug_chunk.draw(m_shader.get(), tile->min, tile->max);
  }
#endif

#if 0
  m_debug_chunk.draw(m_shader.get(), center);
#endif

  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}
