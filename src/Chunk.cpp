#include "Chunk.h"

Chunk::Chunk(size_t size) : m_vao(std::make_unique<VertexArrayObject>()), m_vbo(std::make_unique<VertexBuffer>())
{
  const std::vector<ChunkVertex> vertices = {
      {{0.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},  // bottom left
      {{0.5f, 0.0f, 0.0f}, {1.0f, 0.0f}},  // bottom right
      {{0.5f, 0.5f, 0.0f}, {1.0f, 1.0f}},  // top right

      {{0.5f, 0.5f, 0.0f}, {1.0f, 1.0f}},  // top right
      {{0.0f, 0.5f, 0.0f}, {0.0f, 1.0f}},  // top left
      {{0.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},  // bottom left
  };

  m_vertex_count = vertices.size();

  m_vao->bind();
  m_vbo->bind();
  m_vbo->buffer_data(std::span(vertices));
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(ChunkVertex), (void*)(offsetof(ChunkVertex, pos)));
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(ChunkVertex), (void*)(offsetof(ChunkVertex, uv)));
  glEnableVertexAttribArray(1);
  m_vao->unbind();
}

void Chunk::draw(ShaderProgram* shader, const glm::vec2& position) const
{
  shader->bind();
  // TODO: set uniforms
  m_vao->bind();
  glDrawArrays(GL_TRIANGLES, 0, m_vertex_count);
  m_vao->unbind();
  shader->unbind();
}
