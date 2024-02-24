#version 430
layout (location = 0) in vec3 a_pos;

uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_proj;

out vec3 uv;

void main() {
  uv = a_pos;
  vec4 pos = u_proj * u_view * vec4(a_pos, 1.0);
  gl_Position = pos.xyww;
}