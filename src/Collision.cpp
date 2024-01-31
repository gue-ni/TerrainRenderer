#include "Collision.h"

int intersect_segment_plane(glm::vec3 a, glm::vec3 b, Plane p, float &t, glm::vec3 &q)
{
  // Compute the t value for the directed line ab intersecting the plane
  glm::vec3 ab = b - a;

  t = (p.d - glm::dot(p.n, a)) / glm::dot(p.n, ab);

  // If t in [0..1] compute and return intersection point
  if (t >= 0.0f && t <= 1.0f) {
    q = a + t * ab;
    return true;
  }

  return false;
}