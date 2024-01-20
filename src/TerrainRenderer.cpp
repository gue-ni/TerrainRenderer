#include "TerrainRenderer.h"

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
void main() {
  FragColor = vec4(uv, 0, 1);
}
)";

TerrainRenderer::TerrainRenderer()
    : m_shader(std::make_unique<ShaderProgram>(shader_vert, shader_frag)), m_debug_chunk(5, 0.75f)
{
}

void TerrainRenderer::render(const Camera& camera, const glm::vec2& center)
{
  // TODO:
  // construct QuadTree
  // if changes are necessary, load new tiles
  // render tiles

  if (m_wireframe) {
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  }
#if 0
  auto model = glm::translate(glm::mat4(1.0f), glm::vec3());
auto view = camera.get_view_matrix();
auto proj = camera.get_projection_matrix();
#else
  glm::mat4 model = glm::mat4(1.0f);
  glm::mat4 view = glm::mat4(1.0f);
  glm::mat4 proj = glm::mat4(1.0f);

  model = glm::translate(model, glm::vec3(-0.2f, 0.0f, 0.0f));
  view = glm::translate(view, glm::vec3(0.0f, 0.0f, -3.0f));
  proj = glm::perspective(glm::radians(45.0f), (float)800 / (float)600, 0.1f, 100.0f);
#endif

  m_shader->bind();
  m_shader->set_uniform("model", model);
  m_shader->set_uniform("view", view);
  m_shader->set_uniform("proj", proj);

  m_debug_chunk.draw(m_shader.get(), glm::vec2(0.0f));

  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}
