/*  */
#pragma once

#include "../gfx.h"
#include "Common.h"

using namespace gfx;
using namespace gfx::gl;

class Chunk
{
 public:
  Chunk(unsigned vertex_count, float size = 1.0f);

  void draw(ShaderProgram* shader, const glm::vec2& min, const glm::vec2& max) const;

 private:
  struct Vertex {
    glm::vec3 pos;
    glm::vec2 uv;
  };

  GLsizei m_vertex_count = 0;
  std::unique_ptr<VertexArrayObject> m_vao;
  std::unique_ptr<VertexBuffer> m_vbo;
  std::unique_ptr<ElementBuffer> m_ebo;
};
