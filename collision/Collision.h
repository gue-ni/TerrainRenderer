#pragma once

#include <array>
#include <glm/glm.hpp>
#include <iostream>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/io.hpp>

using Point = glm::vec3;

// An axis aligned bounding box is defined by a minimum and a maximum point.
struct AABB {
  glm::vec3 min, max;
  AABB(const glm::vec3 &min_, const glm::vec3 &max_);
  inline glm::vec3 size() const { return max - min; }
  inline glm::vec3 center() const { return min + size() / 2.0f; }
  std::array<glm::vec3, 8> corners() const;
  static AABB from_center_and_size(const glm::vec3 &center, const glm::vec3 &size);
};

// A ray is defined by an origin and a normalized direction.
struct Ray {
  glm::vec3 origin, direction;
  Ray(const glm::vec3 &origin_, const glm::vec3 &direction_);
  inline glm::vec3 point_at(float t) const { return origin + direction * t; }
};

// A sphere is defined by a center and a radius.
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
  float signed_distance(const Point &) const;
};

inline std::ostream &operator<<(std::ostream &os, const Plane &p) { return os << p.normal << ", " << p.distance; }

// A frustum is defined by 6 planes that each point inwards.
struct Frustum {
  enum : std::size_t { NEAR = 0, FAR, TOP, BOTTOM, LEFT, RIGHT };
  std::array<Plane, 6> planes;
  Frustum(const glm::mat4 &view_projection_matrix);
};

// Return true if ray intersects plane.
bool ray_vs_plane(const Ray &, const Plane &, float &t);

// Return true if ray intersects sphere.
bool ray_vs_sphere(const Ray &, const Sphere &, float &t);

// Return true if point is in front of plane (signed distance is >= 0).
bool point_vs_plane(const Point &, const Plane &);

// Return true if point is inside frustum.
bool point_vs_frustum(const Point &, const Frustum &);

// Return true if spheres intersect.
bool sphere_vs_sphere(const Sphere &, const Sphere &);

// Return true if any corner is in front of plane.
bool aabb_vs_plane(const AABB &, const Plane &);

// Return true if AABBs intersect.
bool aabb_vs_aabb(const AABB &, const AABB &);

// Return true if aabb is (even partly) inside frustum.
bool aabb_vs_frustum(const AABB &, const Frustum &);
