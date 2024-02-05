#pragma once

#include <array>
#include <glm/glm.hpp>

using Point = glm::vec3;

struct AABB {
  glm::vec3 min, max;
  AABB(const glm::vec3 &min_, const glm::vec3 &max_);
  inline glm::vec3 size() const { return max - min; }
  inline glm::vec3 center() const { return min + size() / 2.0f; }
  std::array<glm::vec3, 8> corners() const;
  static AABB from_center_and_size(const glm::vec3 &center, const glm::vec3 &size);
};

struct Ray {
  glm::vec3 origin, direction;
  Ray(const glm::vec3 &origin_, const glm::vec3 &direction_);
  inline glm::vec3 point_at(float t) const { return origin + direction * t; }
};

struct Sphere {
  glm::vec3 center;
  float radius;
  Sphere(const glm::vec3 &center_, float radius_);
};

// A plane is defined by a normal and a distance from the origin.
struct Plane {
  glm::vec3 normal;
  float distance;
  Plane();
  Plane(const glm::vec4 &vec);
  Plane(const glm::vec3 &plane_normal, float distance_from_origin);
  Plane(const glm::vec3 &plane_normal, const glm::vec3 &point_on_plane);
  void normalize();
  float distance_from_plane(const Point &) const;
};

struct Frustum {
  enum : std::size_t { NEAR = 0, FAR, TOP, BOTTOM, LEFT, RIGHT };
  std::array<Plane, 6> planes;
  Frustum(const glm::mat4 &view_projection_matrix);
};

bool ray_vs_plane(const Ray &, const Plane &, float &t);

bool ray_vs_sphere(const Ray &, const Sphere &, float &t);

// Return true if point is behind plane.
bool point_vs_plane(const Point &, const Plane &);

// Return true if point is inside frustum.
bool point_vs_frustum(const Point &, const Frustum &);

bool sphere_vs_sphere(const Sphere &, const Sphere &);

bool aabb_vs_plane(const AABB &, const Plane &);

bool aabb_vs_aabb(const AABB &, const AABB &);

bool aabb_vs_frustum(const AABB &, const Frustum &);
