#pragma once

#include <glm/glm.hpp>

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

struct Plane {
  glm::vec3 n;  // plane normal
  float d;      // d = dot(n, p) for a given point on the plane
  Plane(const glm::vec3 &normal, const glm::vec3 &point) : n(normal), d(glm::dot(normal, point)) {}
};

// A segment is a part of a straigt line bounded by two end points.
struct Segment {
  glm::vec3 a, b;
  inline glm::vec3 point_at(float t) const { return a + (b - a) * t; }
};

// Real Time Collison Detection - Christer Ericson, Page
bool segment_vs_plane(const Segment &segment, const Plane &plane, float &t);

bool ray_vs_sphere(const Ray &ray, const Sphere &sphere, float &t);
