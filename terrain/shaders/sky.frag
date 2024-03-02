#version 430

uniform vec3 u_sky_color;
uniform vec3 u_sun_dir;
uniform vec3 u_sun_color;
uniform vec3 u_light_blue;
uniform vec3 u_dark_blue;
uniform vec3 u_camera_position;
uniform vec3 u_lat_lon_alt;

out vec4 frag_color;

in vec3 uv;

vec3 atmospheric_scattering(vec3 ray_origin, vec3 ray_direction)
{
  vec3 sky_color = mix(u_light_blue, u_dark_blue, ray_direction.y);

  float sun_factor = max(dot(ray_direction, u_sun_dir), 0.0);

  vec3 color = mix(sky_color, u_sun_color, pow(sun_factor, 64.0));

  return color;
}

void main() {
  vec3 camera_dir = normalize(uv);

  vec3 color = atmospheric_scattering(u_camera_position, camera_dir);

  frag_color = vec4(color, 1);
}