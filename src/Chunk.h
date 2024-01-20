/*  */
#pragma once

#include "../gfx/gfx.h"
#include "Common.h"

using namespace gfx::gl;

struct ChunkVertex {
  glm::vec3 pos;
  glm::vec2 uv;
};

inline std::ostream& operator<<(std::ostream& os, const ChunkVertex& v)
{
  os << v.pos << ", " << v.uv;
  return os;
}

class Chunk
{
 public:
  Chunk(uint vertex_count, float size = 1.0f);
  void draw(ShaderProgram* shader, const glm::vec2& position) const;

 private:
  GLsizei m_vertex_count{0};
  std::unique_ptr<VertexArrayObject> m_vao;
  std::unique_ptr<VertexBuffer> m_vbo;
  std::unique_ptr<ElementBuffer> m_ebo;
};
