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
  m_vbo->buffer_data(vertices.data(), vertices.size_bytes());

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
  glEnableVertexAttribArray(0);

  m_vao->unbind();
}

void Cube::draw(ShaderProgram* shader, const glm::vec3& position, float scale) const
{
  shader->bind();

  glm::mat4 model(1.0f);
  model = glm::translate(model, position);
  model = glm::scale(model, glm::vec3(scale));

  shader->set_uniform("u_model", model);

  m_vao->bind();
  glDrawArrays(GL_TRIANGLES, 0, 36);
  m_vao->unbind();
}
