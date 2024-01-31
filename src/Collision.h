#pragma once

#include <glm/glm.hpp>

struct Plane {
  glm::vec3 n;  // plane normal
  float d;      // d = dot(n, p) for a given point on the plane
  Plane(const glm::vec3 &normal, const glm::vec3 &point) : n(normal), d(glm::dot(normal, point)) {}
};

struct AABB {
  glm::vec3 min, max;
};

struct Ray {
  glm::vec3 origin, direction;
};

struct Sphere {
  glm::vec3 center;
  float radius;
};

// Real Time Collison Detection - Christer Ericson
int intersect_segment_plane(glm::vec3 a, glm::vec3 b, Plane p, float &t, glm::vec3 &q);
