#include "Collision.h"

#include <cassert>

Frustum::Frustum(const glm::mat4& view_projection_matrix)
{
  // TODO
}

bool ray_vs_plane(const Ray& ray, const Plane& plane, float& t)
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

bool point_vs_plane(const Point& point, const Plane& plane)
{
  return 0.0f > glm::dot(point, plane.normal) - plane.distance;
}

// stolen from https://github.dev/recp/cglm
bool aabb_vs_frustum(const AABB& aabb, const Frustum& frustum) { return true; }
