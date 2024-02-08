#pragma once

#include "../gfx.h"

using namespace gfx;
using namespace gfx::gl;

class Cube
{
 public:
  Cube();

  void draw(ShaderProgram* shader, const glm::vec3& position, float scale = 1.0f) const;

 private:
  std::unique_ptr<VertexArrayObject> m_vao;
  std::unique_ptr<VertexBuffer> m_vbo;
};