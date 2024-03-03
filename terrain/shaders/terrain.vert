#version 430
layout (location = 0) in vec3 a_pos;
layout (location = 1) in vec2 a_tex;

uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_proj;
uniform float u_height_scaling_factor;
uniform float u_terrain_scaling_factor;
uniform vec2 u_height_uv_min;
uniform vec2 u_height_uv_max;
uniform sampler2D u_height_texture;
uniform uint u_zoom;
uniform float u_pixel_resolution;

out vec2 uv;
out vec4 world_pos;
out vec3 normal;

float altitude_from_color(vec4 color) {
  // depending on encoding
#if 0
  return (color.r + color.g / 255.0);
#else
  return color.r * u_height_scaling_factor;
#endif
}

vec2 map_range(vec2 value, vec2 in_min, vec2 in_max, vec2 out_min, vec2 out_max) {
  return out_min + (value - in_min) * (out_max - out_min) / (in_max - in_min);
}

vec3 compute_normal(vec2 uv) {
  // https://stackoverflow.com/a/5284527/11009152
  // https://stackoverflow.com/a/5282364/11009152

  vec2 size = vec2(u_pixel_resolution, 0.0);

  float h00 = altitude_from_color(textureOffset(u_height_texture, uv, ivec2(-1,0)));
  float h01 = altitude_from_color(textureOffset(u_height_texture, uv, ivec2(+1,0)));
  float h10 = altitude_from_color(textureOffset(u_height_texture, uv, ivec2(0,-1)));
  float h11 = altitude_from_color(textureOffset(u_height_texture, uv, ivec2(0,+1)));

  vec3 va = normalize(vec3(size.x, h00 - h01, size.y));      
  vec3 vb = normalize(vec3(size.y, h10 - h11, -size.x));

  return cross(va, vb);
}

void main() {
  uv = a_tex;

  world_pos = u_model * vec4(a_pos, 1.0);

  vec2 scaled_uv = map_range(uv, vec2(0), vec2(1), u_height_uv_min, u_height_uv_max);

  vec4 height_sample = texture(u_height_texture, scaled_uv);

  normal = compute_normal(scaled_uv);

  float height = altitude_from_color(height_sample) * u_terrain_scaling_factor;

  world_pos.y = height;

  // skirts on tiles
  if (uv.x < 0.0 || uv.x > 1.0 || uv.y < 0.0 || uv.y > 1.0) {
    world_pos.y = -2.0;
  }

  gl_Position = u_proj * u_view * world_pos;
}