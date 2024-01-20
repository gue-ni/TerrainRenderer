/*  */
#pragma once

#include "../gfx/gfx.h"

using namespace gfx::gl;

struct ChunkVertex {
  glm::vec3 pos;
  glm::vec2 uv;
};

class Chunk
{
 public:
  Chunk(size_t size);
  void draw(ShaderProgram* shader, const glm::vec2& position) const;

 private:
  GLsizei m_vertex_count{0};
  std::unique_ptr<VertexArrayObject> m_vao;
  std::unique_ptr<VertexBuffer> m_vbo;
};
