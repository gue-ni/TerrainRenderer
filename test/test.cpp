#include <catch2/catch_test_macros.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/io.hpp>

#include "Collision.h"

TEST_CASE("AABB corners") { AABB aabb(glm::vec3(0.0f), glm::vec3(1.0f)); }

TEST_CASE("Point vs Plane")
{
  Plane plane(glm::vec3(0.0f, 1.0f, 0.0f), 0.0f);

  SECTION("in front of plane")
  {
    Point point(0.0f, 2.0f, 0.0f);
    REQUIRE(point_vs_plane(point, plane) == false);
  }

  SECTION("on plane")
  {
    Point point(0.0f, 0.0f, 0.0f);
    REQUIRE(point_vs_plane(point, plane) == true);
  }

  SECTION("behind plane")
  {
    Point point(0.0f, -2.0f, 0.0f);
    REQUIRE(point_vs_plane(point, plane) == true);
  }
}

TEST_CASE("AABB vs Plane")
{
  Plane plane(glm::vec3(0.0f, 1.0f, 0.0f), 0.0f);

  {
    AABB aabb(glm::vec3(-1.0f), glm::vec3(1.0f));
    REQUIRE(aabb_vs_plane(aabb, plane) == true);
  }

  {
    AABB aabb = AABB::from_center_and_size(glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(1.0f));
    REQUIRE(aabb_vs_plane(aabb, plane) == false);
  }
}

TEST_CASE("AABB vs Frustum")
{
  glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f));
  glm::mat4 proj = glm::perspective(glm::radians(45.0f), 16.0f / 9.0f, 1.0f, 1000.0f);
  glm::mat4 view_proj = proj * view;

  Frustum frustum(view_proj);

  SECTION("test frustum creation")
  {
    for (auto& plane : frustum.planes) {
      std::cout << "normal: " << plane.normal << ", distance: " << plane.distance << std::endl;
    }
  }

  SECTION("aabb inside frustum")
  {
    AABB aabb = AABB::from_center_and_size(glm::vec3(0.0f, 0.0f, 5.0f), glm::vec3(1.0f));
    REQUIRE(aabb_vs_frustum(aabb, frustum) == true);
  }

  SECTION("aabb behind camera")
  {
    glm::vec3 center = glm::vec3(0.0f, 0.0f, -5.0f);
    AABB aabb = AABB::from_center_and_size(center, glm::vec3(1.0f));
    std::cout << aabb.min << aabb.max << std::endl;

    // should be behind near plane
    REQUIRE(point_vs_plane(center, frustum.planes[Frustum::NEAR]) == true);

    REQUIRE(aabb_vs_plane(aabb, frustum.planes[Frustum::NEAR]) == true);

    // REQUIRE(aabb_vs_frustum(aabb, frustum) == false);
  }
}