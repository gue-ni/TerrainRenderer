#include "Cube.h"

Cube::Cube() : m_vao(std::make_unique<VertexArrayObject>()), m_vbo(std::make_unique<VertexBuffer>())

{
  const std::vector<glm::vec3> vertices = {
      {-1.0f, 1.0f, -1.0f},  {-1.0f, -1.0f, -1.0f}, {1.0f, -1.0f, -1.0f},  {1.0f, -1.0f, -1.0f}, {1.0f, 1.0f, -1.0f},
      {-1.0f, 1.0f, -1.0f},  {-1.0f, -1.0f, 1.0f},  {-1.0f, -1.0f, -1.0f}, {-1.0f, 1.0f, -1.0f}, {-1.0f, 1.0f, -1.0f},
      {-1.0f, 1.0f, 1.0f},   {-1.0f, -1.0f, 1.0f},  {1.0f, -1.0f, -1.0f},  {1.0f, -1.0f, 1.0f},  {1.0f, 1.0f, 1.0f},
      {1.0f, 1.0f, 1.0f},    {1.0f, 1.0f, -1.0f},   {1.0f, -1.0f, -1.0f},  {-1.0f, -1.0f, 1.0f}, {-1.0f, 1.0f, 1.0f},
      {1.0f, 1.0f, 1.0f},    {1.0f, 1.0f, 1.0f},    {1.0f, -1.0f, 1.0f},   {-1.0f, -1.0f, 1.0f}, {-1.0f, 1.0f, -1.0f},
      {1.0f, 1.0f, -1.0f},   {1.0f, 1.0f, 1.0f},    {1.0f, 1.0f, 1.0f},    {-1.0f, 1.0f, 1.0f},  {-1.0f, 1.0f, -1.0f},
      {-1.0f, -1.0f, -1.0f}, {-1.0f, -1.0f, 1.0f},  {1.0f, -1.0f, -1.0f},  {1.0f, -1.0f, -1.0f}, {-1.0f, -1.0f, 1.0f},
      {1.0f, -1.0f, 1.0f}};

  m_vao->bind();

  m_vbo->bind();
  m_vbo->buffer_data(std::span(vertices));

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
  glEnableVertexAttribArray(0);

  m_vao->unbind();
}

void Cube::draw(ShaderProgram* shader, const glm::vec3& position, float scale) const
{
  shader->bind();

  auto model = glm::translate(glm::mat4(1.0f), position) * glm::scale(glm::mat4(1.0f), glm::vec3(scale));
  shader->set_uniform("u_model", model);

  m_vao->bind();
  glDrawArrays(GL_TRIANGLES, 0, 36);
  m_vao->unbind();
}
