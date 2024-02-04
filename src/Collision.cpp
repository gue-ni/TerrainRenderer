#include "Collision.h"

#include <cassert>

Plane::Plane() : normal({0.0f, 1.0f, 0.0f}), distance(0.0f) {}

Plane::Plane(const glm::vec4& vec) : normal(glm::vec3(vec)), distance(vec.w) {}

Plane::Plane(const glm::vec3& plane_normal, float distance_from_origin)
    : normal(plane_normal), distance(distance_from_origin)
{
}

Plane::Plane(const glm::vec3& plane_normal, const glm::vec3& point_on_plane)
    : normal(plane_normal), distance(glm::dot(plane_normal, point_on_plane))
{
}

void Plane::normalize() { normal = glm::normalize(normal); }

Frustum::Frustum(const glm::mat4& view_projection_matrix)
{
  glm::mat4 transposed = glm::transpose(view_projection_matrix);

  planes[LEFT] = Plane(transposed[3] + transposed[0]);
  planes[RIGHT] = Plane(transposed[3] - transposed[0]);
  planes[BOTTOM] = Plane(transposed[3] - transposed[1]);
  planes[TOP] = Plane(transposed[3] + transposed[1]);
  planes[NEAR] = Plane(transposed[3] + transposed[2]);
  planes[FAR] = Plane(transposed[3] - transposed[2]);

  for (auto& plane : planes) plane.normalize();
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
  // TODO
  return false;
}

bool point_vs_plane(const Point& point, const Plane& plane)
{
  return 0.0f > glm::dot(point, plane.normal) - plane.distance;
}

// https://github.dev/recp/cglm
// https://cgvr.cs.uni-bremen.de/teaching/cg_literatur/lighthouse3d_view_frustum_culling/index.html
bool aabb_vs_frustum(const AABB& aabb, const Frustum& frustum)
{
  // TODO
  return true;
}
