#version 430

uniform vec3 u_sky_color;
uniform vec3 u_sun_dir;
uniform vec3 u_sun_color;
uniform vec3 u_camera_position;

out vec4 frag_color;

in vec3 uv;

vec3 atmospheric_scattering(
  in vec3 ray_origin,
  in vec3 ray_direction
) 
{



  return vec3(1.0, 0.0, 0.0);
}

void main() {
#if 1
  vec3 camera_dir = normalize(uv);

  float sun_factor = max(dot(camera_dir, u_sun_dir), 0.0);

  vec3 color = mix(u_sky_color, u_sun_color, pow(sun_factor, 64.0));

  frag_color = vec4(color, 1);
#else
  vec3 color = atmospheric_scattering();
  frag_color = vec4(color, 1);
#endif
}