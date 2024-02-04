#pragma once

#include <array>
#include <glm/glm.hpp>

using Point = glm::vec3;

struct AABB {
  glm::vec3 min, max;
  inline glm::vec3 size() const { return max - min; }
  inline glm::vec3 center() const { return min + size() / 2.0f; }
};

struct Ray {
  glm::vec3 origin, direction;
  inline glm::vec3 point_at(float t) const { return origin + direction * t; }
};

struct Sphere {
  glm::vec3 center;
  float radius;
};

// A plane is defined by a normal and a distance from the origin.
struct Plane {
  glm::vec3 normal;
  float distance;
  Plane() : normal({0.0f, 1.0f, 0.0f}), distance(0.0f) {}
  Plane(const glm::vec3 &plane_normal, const glm::vec3 &point_on_plane)
      : normal(plane_normal), distance(glm::dot(plane_normal, point_on_plane))
  {
  }
};

struct Frustum {
  std::array<Plane, 6> planes;
  Frustum(const glm::mat4 &view_projection_matrix);
};

bool ray_vs_plane(const Ray &, const Plane &, float &t);

bool ray_vs_sphere(const Ray &, const Sphere &, float &t);

// Return true if point is behind plane.
bool point_vs_plane(const Point &, const Plane &);

bool aabb_vs_plane(const AABB &, const Plane &);

bool aabb_vs_frustum(const AABB &, const Frustum &);
