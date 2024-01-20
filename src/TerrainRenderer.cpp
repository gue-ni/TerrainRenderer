#include "TerrainRenderer.h"

const std::string shader_vert = R"(
#version 430
layout (location = 0) in vec3 a_pos;
layout (location = 1) in vec2 a_tex;

uniform mat4 model, view, projection;

out vec2 uv;

void main() {
  uv = a_tex;
  gl_Position = vec4(a_pos, 1);
}
)";

const std::string shader_frag = R"(
#version 430
out vec4 FragColor;
in vec2 uv;
void main() {
  FragColor = vec4(uv, 0, 1);
}
)";

TerrainRenderer::TerrainRenderer()
    : m_shader(std::make_unique<ShaderProgram>(shader_vert, shader_frag)), m_debug_chunk(3)
{
}

void TerrainRenderer::render(const glm::vec2& center)
{
  // TODO:
  // construct QuadTree
  // if changes are necessary, load new tiles
  // render tiles

  if (m_wireframe) {
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  }

  m_debug_chunk.draw(m_shader.get(), glm::vec2(0.0f));

  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}
