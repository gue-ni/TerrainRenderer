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
  std::array<glm::vec3, 8> vertices() const;
  bool contains(const AABB &);
  static AABB from_center_and_size(const glm::vec3 &center, const glm::vec3 &size);

  template <class It>
  static AABB from_points(It begin, It end)
  {
    using limits = std::numeric_limits<float>;
    glm::vec3 min(limits::max()), max(limits::min());

    for (auto it = begin; it != end; ++it) {
      min = glm::min(min, *it);
      max = glm::max(max, *it);
    }

    return AABB(min, max);
  }
};

// A ray is defined by an origin and a normalized direction.
struct Ray {
  glm::vec3 origin, direction;
  Ray(const glm::vec3 &origin_, const glm::vec3 &direction_);
  inline glm::vec3 point_at(float t) const { return origin + direction * t; }
  static Ray between_points(const glm::vec3 &source, const glm::vec3 &target);
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

// A frustum is defined by 6 planes that each point inwards.
struct Frustum {
  enum : std::size_t { NEAR = 0, FAR, TOP, BOTTOM, LEFT, RIGHT };
  std::array<Plane, 6> planes;
  Frustum(const glm::mat4 &view_projection_matrix);
  Plane &right() { return planes[RIGHT]; }
  Plane &left() { return planes[LEFT]; };
  Plane &near() { return planes[NEAR]; }
  Plane &far() { return planes[FAR]; }
  Plane &top() { return planes[TOP]; }
  Plane &bottom() { return planes[BOTTOM]; }
  // this does not work properly
  std::array<glm::vec3, 8> vertices() const;
};

inline std::ostream &operator<<(std::ostream &os, const Plane &p) { return os << p.normal << ", " << p.distance; }

inline std::ostream &operator<<(std::ostream &os, const AABB &a) { return os << a.min << " " << a.max; }

inline std::ostream &operator<<(std::ostream &os, const Frustum &f)
{
  os << "near:    " << f.planes[Frustum::NEAR] << "\n";
  os << "far:     " << f.planes[Frustum::FAR] << "\n";
  os << "left:    " << f.planes[Frustum::LEFT] << "\n";
  os << "right:   " << f.planes[Frustum::RIGHT] << "\n";
  os << "top:     " << f.planes[Frustum::TOP] << "\n";
  os << "bottom:  " << f.planes[Frustum::BOTTOM] << "\n";
  return os;
}

// Return true and point if the 3 planes intersect.
bool plane_vs_plane_vs_plane(const Plane &, const Plane &, const Plane &, Point &);

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
