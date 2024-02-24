#version 430

uniform vec3 u_sky_color_1;
uniform vec3 u_sky_color_2;

out vec4 frag_color;
in vec3 uv;

#define sq(x) (x * x)

vec3 map_cube_to_sphere(vec3 point_on_cube) {
    float x = point_on_cube.x, y = point_on_cube.y, z = point_on_cube.z;

    vec3 point_on_sphere;
    point_on_sphere.x = x * sqrt(1.0f - (sq(y) / 2.0f) - (sq(z) / 2.0f) + ((sq(y) * sq(z)) / 3.0f));
    point_on_sphere.y = y * sqrt(1.0f - (sq(z) / 2.0f) - (sq(x) / 2.0f) + ((sq(z) * sq(x)) / 3.0f));
    point_on_sphere.z = z * sqrt(1.0f - (sq(x) / 2.0f) - (sq(y) / 2.0f) + ((sq(x) * sq(y)) / 3.0f));
    return point_on_sphere;
}

void main() {
  vec3 spherical = map_cube_to_sphere(uv);

  vec3 color = mix(u_sky_color_1, u_sky_color_2, spherical.y);

  vec3 sun_dir = normalize(vec3(0, 1, 2));
  vec3 sun_color = vec3(1.0, 0.9, 0.7);
  float sun_factor = max(dot(spherical, sun_dir), 0.0);

  color  = mix(color, sun_color, pow(sun_factor, 8.0));

  frag_color = vec4(color, 1);
}