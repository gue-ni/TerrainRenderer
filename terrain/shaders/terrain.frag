#version 430

in vec2 uv;
in vec4 world_pos;
out vec3 out_normal;

out vec4 frag_color;

uniform vec2 u_albedo_uv_min;
uniform vec2 u_albedo_uv_max;
uniform sampler2D u_albedo_texture;
uniform vec3 u_fog_color;
uniform float u_fog_near;
uniform float u_fog_far;
uniform float u_fog_density;
uniform vec3 u_camera_position;
uniform uint u_zoom;
uniform bool u_debug_view;

uint compute_hash(uint a) {
   uint b = (a+2127912214u) + (a<<12u);
   b = (b^3345072700u) ^ (b>>19u);
   b = (b+374761393u) + (b<<5u);
   b = (b+3551683692u) ^ (b<<9u);
   b = (b+4251993797u) + (b<<3u);
   b = (b^3042660105u) ^ (b>>16u);
   return b;
}

vec3 color_from_uint(uint a) {
    uint hash = compute_hash(a);
    return vec3(float(hash & 255u), float((hash >> 8u) & 255u), float((hash >> 16u) & 255u)) / 255.0;
}

vec2 map_range(vec2 value, vec2 in_min, vec2 in_max, vec2 out_min, vec2 out_max) {
  return out_min + (value - in_min) * (out_max - out_min) / (in_max - in_min);
}

void main() {
  vec2 scaled_uv = map_range(uv, vec2(0), vec2(1), u_albedo_uv_min, u_albedo_uv_max);

  vec3 color = texture(u_albedo_texture, scaled_uv).rgb;

#if 0
  color = mix(color, vec3(scaled_uv.xy, 0), 0.5);
#endif

  if (u_fog_color != vec3(0)) {
    vec3 camera_dir = normalize(u_camera_position - world_pos.xyz);
    float camera_dist = length(world_pos.xyz - u_camera_position);
    float dist_ratio = 4.0 * camera_dist / u_fog_far;
    float fog_factor = 1.0 - exp(-dist_ratio * u_fog_density);

    vec3 sun_dir = normalize(vec3(0, 1, 2));
    vec3 sun_color = vec3(1.0, 0.9, 0.7);
    float sun_factor = max(dot(camera_dir, sun_dir), 0.0);

    //vec3 fog_color  = mix(u_fog_color, sun_color, pow(sun_factor, 8.0));
    vec3 fog_color = u_fog_color;

    color = mix(color, fog_color, fog_factor);
  }

  if (u_debug_view) {
    color = mix(color_from_uint(u_zoom), color, 0.5);
  }

  // color = out_normal;

  frag_color = vec4(color, 1);
}