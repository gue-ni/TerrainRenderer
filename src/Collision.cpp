#include "Collision.h"

#include <cassert>

bool segment_vs_plane(const Segment& segment, const Plane& plane, float& t)
{
  glm::vec3 ab = segment.b - segment.a;

  t = (plane.d - glm::dot(plane.n, segment.a)) / glm::dot(plane.n, ab);

  if (0.0f <= t && t <= 1.0f) {
    return true;
  }

  return false;
}

bool ray_vs_sphere(const Ray& ray, const Sphere& sphere, float& t)
{
  assert(false);
  return false;
}
