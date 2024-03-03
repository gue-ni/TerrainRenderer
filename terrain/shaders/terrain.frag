#version 430

in vec2 uv;
in vec4 world_pos;
in vec3 normal;

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
uniform bool u_shading;
uniform vec3 u_sun_dir;
uniform vec3 u_sun_color;
uniform vec3 u_light_blue;
uniform vec3 u_dark_blue;

uint compute_hash(uint a) {
   uint b = (a+2127912214u) + (a<<12u);
   b = (b^3345072700u) ^ (b>>19u);
   b = (b+374761393u) + (b<<5u);
   b = (b+3551683692u) ^ (b<<9u);
   b = (b+4251993797u) + (b<<3u);
   b = (b^3042660105u) ^ (b>>16u);
   return b;
}

vec3 shading(
  in vec3 albedo,
  in vec3 norm,
  in vec3 sun_dir,
  in vec3 sun_color
) {
  float ambient_strength = 0.3;
  vec3 ambient = ambient_strength * sun_color;

  float n_dot_l = max(0.0, dot(norm, sun_dir)); // lambertian coefficient
  vec3 diffuse = n_dot_l * sun_color;

  return (ambient + diffuse) * albedo;
}

vec3 color_from_uint(uint a) {
    uint hash = compute_hash(a);
    return vec3(float(hash & 255u), float((hash >> 8u) & 255u), float((hash >> 16u) & 255u)) / 255.0;
}

vec2 map_range(vec2 value, vec2 in_min, vec2 in_max, vec2 out_min, vec2 out_max) {
  return out_min + (value - in_min) * (out_max - out_min) / (in_max - in_min);
}

vec3 visualize_normal(vec3 n) {
    return (n + vec3(1.0)) / vec3(2.0);
}

void main() {
  vec2 scaled_uv = map_range(uv, vec2(0), vec2(1), u_albedo_uv_min, u_albedo_uv_max);

  vec3 color = texture(u_albedo_texture, scaled_uv).rgb;

#if 0
  color = mix(color, vec3(scaled_uv.xy, 0), 0.5);
#endif

  if (u_debug_view) {
    color = mix(color_from_uint(u_zoom), color, 0.5);
  }

#if 0
  color = mix(color, visualize_normal(normal), 0.5);
#endif
#if 1
  if (u_shading) {
   color = shading(
      color,
      normal,
      u_sun_dir,
      u_sun_color
    );
  }
#endif
#if 1
  vec3 camera_dir = normalize(u_camera_position - world_pos.xyz);
  float camera_dist = length(world_pos.xyz - u_camera_position);
  float dist_ratio = 4.0 * camera_dist / u_fog_far;
  float fog_factor = 1.0 - exp(-dist_ratio * u_fog_density);

  vec3 sky_color = mix(u_light_blue, u_dark_blue, camera_dir.y);

  float sun_factor = max(dot(-camera_dir, u_sun_dir), 0.0);

  vec3 fog_color  = mix(sky_color, u_sun_color, pow(sun_factor, 32.0));

  color = mix(color, fog_color, fog_factor);
#endif

  frag_color = vec4(color, 1);
}