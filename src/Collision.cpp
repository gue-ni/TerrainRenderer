#include "Collision.h"

#include <cassert>

bool segment_vs_plane(const Segment& segment, const Plane& plane, float& t)
{
  glm::vec3 ab = segment.b - segment.a;

  t = (plane.distance - glm::dot(plane.normal, segment.a)) / glm::dot(plane.normal, ab);

  if (0.0f <= t && t <= 1.0f) {
    return true;
  }

  return false;
}

bool ray_vs_plane(const Ray& ray, const Plane& plane, float &t)
{
  float nd = glm::dot(ray.direction, plane.normal);
  float pn = glm::dot(ray.origin, plane.normal);

  if (0.0f <= nd) {
    return false;
  }

  t = (plane.distance - pn) / nd;

  return 0.0f <= t;
}

bool ray_vs_sphere(const Ray& ray, const Sphere& sphere, float& t)
{
  assert(false);
  return false;
}
