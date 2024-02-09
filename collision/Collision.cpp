#include "Collision.h"

AABB::AABB(const glm::vec3& min_, const glm::vec3& max_) : min(min_), max(max_) {}

AABB AABB::from_center_and_size(const glm::vec3& center, const glm::vec3& size)
{
  glm::vec3 half_size = size / 2.0f;
  return AABB(center - half_size, center + half_size);
}

Ray::Ray(const glm::vec3& origin_, const glm::vec3& direction_) : origin(origin_), direction(direction_) {}

Ray Ray::between_points(const glm::vec3& source, const glm::vec3& target)
{
  return Ray(source, glm::normalize(target - source));
}

Sphere::Sphere(const glm::vec3& center_, float radius_) : center(center_), radius(radius_) {}

std::array<glm::vec3, 8> AABB::corners() const
{
  return {
      glm::vec3(min.x, min.y, min.z), glm::vec3(max.x, min.y, min.z),
      glm::vec3(min.z, min.y, max.z), glm::vec3(max.x, min.y, max.z),

      glm::vec3(min.x, max.y, min.z), glm::vec3(max.x, max.y, min.z),
      glm::vec3(min.z, max.y, max.z), glm::vec3(max.x, max.y, max.z),
  };
}

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

void Plane::normalize()
{
  float length = glm::length(normal);
  normal /= length;
  distance /= length;
}

float Plane::signed_distance(const Point& point) const { return glm::dot(point, normal) - distance; }

Frustum::Frustum(const glm::mat4& view_projection_matrix)
{
  glm::mat4 transposed = glm::transpose(view_projection_matrix);

  planes[LEFT] = Plane(transposed[3] + transposed[0]);
  planes[RIGHT] = Plane(transposed[3] - transposed[0]);
  planes[TOP] = Plane(transposed[3] + transposed[1]);
  planes[BOTTOM] = Plane(transposed[3] - transposed[1]);
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

bool point_vs_plane(const Point& point, const Plane& plane) { return 0.0f <= plane.signed_distance(point); }

bool point_vs_frustum(const Point& point, const Frustum& frustum)
{
  for (const Plane& plane : frustum.planes) {
    // point is behind plane
    if (!point_vs_plane(point, plane)) {
      return false;
    }
  }
  return true;
}

bool sphere_vs_sphere(const Sphere& a, const Sphere& b)
{
  return glm::distance(a.center, b.center) <= (a.radius + b.radius);
}

bool aabb_vs_aabb(const AABB& a, const AABB& b)
{
  return glm::all(glm::greaterThanEqual(a.min, b.max)) && glm::all(glm::greaterThanEqual(b.min, a.max));
}

bool aabb_vs_plane(const AABB& aabb, const Plane& plane)
{
  for (const glm::vec3& corner : aabb.corners()) {
    if (0.0f <= plane.signed_distance(corner)) {
      return true;
    }
  }
  return false;
}

// https://cgvr.cs.uni-bremen.de/teaching/cg_literatur/lighthouse3d_view_frustum_culling/index.html
bool aabb_vs_frustum(const AABB& aabb, const Frustum& frustum)
{
  auto vertices = aabb.corners();
#if 0
  for (auto& plane : frustum.planes) {
    int front = 0, behind = 0;

    for (int k = 0; k < 8 && (front == 0 || behind == 0); k++) {
      if (plane.signed_distance(vertices[k]) < 0.0f) {
        behind++;
      } else {
        front++;
      }
    }

    if (front == 0) {
      return false;  // all corners are outside
    }
  }
  return true;
#else

  for (const Plane& plane : frustum.planes) {
    if (!aabb_vs_plane(aabb, plane)) {
      return false;
    }
  }
  return true;
#endif
}
