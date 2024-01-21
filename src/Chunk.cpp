#include "Chunk.h"

#include <algorithm>
#include <iostream>

auto print = [](const auto& v) { std::cout << v << std::endl; };

Chunk::Chunk(uint vertex_count, float size)
    : m_vao(std::make_unique<VertexArrayObject>()),
      m_vbo(std::make_unique<VertexBuffer>()),
      m_ebo(std::make_unique<ElementBuffer>())
{
  assert(vertex_count >= 2);
#if 0
  const std::vector<ChunkVertex> vertices = {
      {{0.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},  // bottom left
      {{0.5f, 0.0f, 0.0f}, {1.0f, 0.0f}},  // bottom right
      {{0.0f, 0.5f, 0.0f}, {0.0f, 1.0f}},  // top left
      {{0.5f, 0.5f, 0.0f}, {1.0f, 1.0f}},  // top right
  };

  const std::vector<unsigned> indices = {
    0, 1, 3, 
    3, 2, 0
  };
#else
  std::vector<ChunkVertex> vertices;
  std::vector<uint> indices;

  float stride = size / (vertex_count - 1);
  auto dimensions = glm::vec2(stride * (vertex_count - 1));

  for (uint y = 0; y < vertex_count; ++y) {
    for (uint x = 0; x < vertex_count; ++x) {
      auto pos = glm::vec3(x * stride, y * stride, 0.0f);
      auto uv = glm::vec2(pos.x, pos.y) / dimensions;
      ChunkVertex vertex = {pos, uv};
      vertices.push_back(vertex);
    }
  }

  for (unsigned x = 0; x < vertex_count - 1; ++x) {
    for (unsigned y = 0; y < vertex_count - 1; ++y) {

      auto bottom_left = (y * vertex_count) + x;
      auto bottom_right = bottom_left + 1;
      auto top_left = ((y + 1) * vertex_count) + x;
      auto top_right = top_left + 1;

      indices.insert(indices.end(), {
                                        bottom_left, bottom_right, top_right,  // triangle 1
                                        top_right, top_left, bottom_left       // triangle 2
                                    });
    }
  }

#endif

  m_vertex_count = static_cast<GLsizei>(indices.size());

  m_vao->bind();

  m_vbo->bind();
  m_vbo->buffer_data(std::span(vertices));

  m_ebo->bind();
  m_ebo->buffer_data(std::span(indices));

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(ChunkVertex), (void*)(offsetof(ChunkVertex, pos)));
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(ChunkVertex), (void*)(offsetof(ChunkVertex, uv)));
  glEnableVertexAttribArray(1);
  m_vao->unbind();
}

void Chunk::draw(ShaderProgram* shader, const glm::vec2& position) const
{
  shader->bind();
  auto model = glm::translate(glm::mat4(1.0f), glm::vec3(position.x, 0.0f, position.y));
  shader->set_uniform("model", model);

  m_vao->bind();
  glDrawElements(GL_TRIANGLES, m_vertex_count, GL_UNSIGNED_INT, 0);
  m_vao->unbind();
  shader->unbind();
}

void Chunk::draw(ShaderProgram* shader, const glm::vec2& min, const glm::vec2& max) const {

  shader->bind();
  //auto model = glm::translate(glm::mat4(1.0f), );
  //auto scale = glm::scale(glm::mat4(1.0f), );

  glm::mat4 translate = glm::translate(glm::mat4(1.0f), glm::vec3(min.x, min.y, 0.0f));
  glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(max.x - min.x));
  auto model = translate *  scale;

  shader->set_uniform("model", model);

  m_vao->bind();
  glDrawElements(GL_TRIANGLES, m_vertex_count, GL_UNSIGNED_INT, 0);
  m_vao->unbind();
  shader->unbind();

}
