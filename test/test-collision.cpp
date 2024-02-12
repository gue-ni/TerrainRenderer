#include <catch2/catch_test_macros.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/io.hpp>

#include "Collision.h"

TEST_CASE("Ray vs Plane")
{
  Plane plane(glm::vec3(0.0f, 1.0f, 0.0f), 0.0f);

  SECTION("parallel to plane")
  {
    float t;
    Ray ray(glm::vec3(0.0f, 2.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    REQUIRE(ray_vs_plane(ray, plane, t) == false);
  }

  SECTION("above plane, looking down")
  {
    float t;
    Ray ray = Ray::between_points(glm::vec3(0.0f, 3.0f, 1.0f), glm::vec3(2.0f, -1.0f, 4.0f));
    REQUIRE(ray_vs_plane(ray, plane, t) == true);
  }

  SECTION("above plane, looking up")
  {
    float t;
    Ray ray = Ray::between_points(glm::vec3(0.0f, 3.0f, 1.0f), glm::vec3(2.0f, 10.0f, 4.0f));
    REQUIRE(ray_vs_plane(ray, plane, t) == false);
  }

  SECTION("below plane, looking up")
  {
    float t;
    Ray ray = Ray::between_points(glm::vec3(0.0f, -3.0f, 1.0f), glm::vec3(2.0f, 10.0f, 4.0f));
    REQUIRE(ray_vs_plane(ray, plane, t) == false);
  }

  SECTION("below plane, looking down")
  {
    float t;
    Ray ray = Ray::between_points(glm::vec3(0.0f, -3.0f, 1.0f), glm::vec3(2.0f, -10.0f, 4.0f));
    REQUIRE(ray_vs_plane(ray, plane, t) == false);
  }
}

TEST_CASE("Point vs Plane")
{
  Plane plane(glm::vec3(0.0f, 1.0f, 0.0f), 0.0f);

  SECTION("point in front of plane")
  {
    Point point(-5.0f, 2.0f, 1.0f);
    REQUIRE(point_vs_plane(point, plane) == true);
  }

  SECTION("point on plane")
  {
    Point point(2.0f, 0.0f, -1.0f);
    REQUIRE(point_vs_plane(point, plane) == true);
  }

  SECTION("point behind plane")
  {
    Point point(1.5f, -2.0f, 6.0f);
    REQUIRE(point_vs_plane(point, plane) == false);
  }
}

TEST_CASE("AABB vs Plane")
{
  Plane plane(glm::vec3(0.0f, 1.0f, 0.0f), 0.0f);

  SECTION("fully above plane")
  {
    AABB aabb = AABB::from_center_and_size(glm::vec3(0.0f, 2.0f, 0.0f), glm::vec3(1.0f));
    REQUIRE(aabb_vs_plane(aabb, plane) == true);
  }

  SECTION("partly above, partly below plane")
  {
    AABB aabb = AABB::from_center_and_size(glm::vec3(0.0f), glm::vec3(1.0f));
    REQUIRE(aabb_vs_plane(aabb, plane) == true);
  }

  SECTION("fully below plane")
  {
    AABB aabb = AABB::from_center_and_size(glm::vec3(0.0f, -5.0f, 0.0f), glm::vec3(1.0f));
    REQUIRE(aabb_vs_plane(aabb, plane) == false);
  }
}

TEST_CASE("Plane vs Plane vs Plane")
{
  SECTION("basic intersection")  
  {
    Plane p0(glm::vec3(1.0f, 0.0f, 0.0f), 0.0f);
    Plane p1(glm::vec3(0.0f, 1.0f, 0.0f), 0.0f);
    Plane p2(glm::vec3(0.0f, 0.0f, 1.0f), 0.0f);

    glm::vec3 point;

    REQUIRE(plane_vs_plane_vs_plane(p0, p1, p2, point) == true);
    // REQUIRE(point == glm::vec3(0.0f));
  }
}

TEST_CASE("Frustum")
{
  // camera looks down the negative z-axis
  glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
  glm::mat4 proj = glm::perspective(glm::radians(45.0f), 16.0f / 9.0f, 1.0f, 1000.0f);
  glm::mat4 view_proj = proj * view;

  Frustum frustum(view_proj);

  SECTION("frustum vertices")
  {
    auto vertices = frustum.vertices();
  }
}

TEST_CASE("AABB vs Frustum")
{
  // camera looks down the negative z-axis
  glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f));
  glm::mat4 proj = glm::perspective(glm::radians(45.0f), 16.0f / 9.0f, 1.0f, 1000.0f);
  glm::mat4 view_proj = proj * view;

  Frustum frustum(view_proj);

  SECTION("test frustum creation")
  {
    for (auto& plane : frustum.planes) {
      std::cout << plane << std::endl;
    }
  }

  SECTION("aabb behind camera")
  {
    glm::vec3 center = glm::vec3(0.0f, 0.0f, 5.0f);
    AABB aabb = AABB::from_center_and_size(center, glm::vec3(1.0f));
    std::cout << aabb.min << aabb.max << std::endl;

    REQUIRE(point_vs_plane(center, frustum.planes[Frustum::NEAR]) == false);
  }

  SECTION("aabb in front of camera")
  {
    glm::vec3 center = glm::vec3(0.0f, 0.0f, -5.0f);
    AABB aabb = AABB::from_center_and_size(center, glm::vec3(1.0f));
    std::cout << aabb.min << aabb.max << std::endl;

    REQUIRE(point_vs_plane(center, frustum.planes[Frustum::NEAR]) == true);
  }
}