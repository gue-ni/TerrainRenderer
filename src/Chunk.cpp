#include "Chunk.h"

#include <algorithm>
#include <iostream>

#define SKIRTS 1

Chunk::Chunk(unsigned vertex_count, float size)
    : m_vao(std::make_unique<VertexArrayObject>()),
      m_vbo(std::make_unique<VertexBuffer>()),
      m_ebo(std::make_unique<ElementBuffer>())
{
  assert(vertex_count >= 2);

  std::vector<ChunkVertex> vertices;
  std::vector<unsigned> indices;

  float stride = size / (vertex_count - 1);
  auto dimensions = glm::vec2(stride * (vertex_count - 1));

  // add padding for skirts
  unsigned vertex_count_no_padding = vertex_count;
#if SKIRTS
  vertex_count += 2;
#endif

  unsigned max_vertex = vertex_count - 1;  // with padding
  unsigned max_clamped_vertex = max_vertex - 1;

  for (unsigned y = 0; y < vertex_count; ++y) {
    for (unsigned x = 0; x < vertex_count; ++x) {
      bool on_border = false;

#if SKIRTS
      if (x == 0 || y == 0 || x == max_vertex || y == max_vertex) {
        on_border = true;
      }

      auto tmp_x = glm::clamp(x, 1u, max_clamped_vertex);
      tmp_x = map_range(tmp_x, 1u, max_clamped_vertex, 0u, vertex_count_no_padding - 1u);

      auto tmp_y = glm::clamp(y, 1u, max_clamped_vertex);
      tmp_y = map_range(tmp_y, 1u, max_clamped_vertex, 0u, vertex_count_no_padding - 1u);
#else
      auto tmp_x = x;
      auto tmp_y = y;
#endif

      auto pos = glm::vec3(tmp_x * stride, 0.0f, tmp_y * stride);
      auto uv = glm::vec2(pos.x, pos.z) / dimensions;

      if (on_border) {
        uv = map_range(uv, glm::vec2(0.0f), glm::vec2(1.0f), glm::vec2(-0.1f), glm::vec2(1.1f));
      }

      vertices.push_back({pos, uv});
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

void Chunk::draw(ShaderProgram* shader, const glm::vec2& min, const glm::vec2& max) const
{
  shader->bind();

  glm::mat4 translate = glm::translate(glm::mat4(1.0f), glm::vec3(min.x, 0.0f, min.y));
  glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(max.x - min.x));
  auto model = translate * scale;

  shader->set_uniform("model", model);

  m_vao->bind();
  glDrawElements(GL_TRIANGLES, m_vertex_count, GL_UNSIGNED_INT, 0);
  m_vao->unbind();
}
