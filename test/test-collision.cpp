#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/io.hpp>

#include "Collision.h"

bool equal(const glm::vec3& a, const glm::vec3& b) { return glm::all(glm::epsilonEqual(a, b, 0.001f)); }

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
  SECTION("point in front of plane")
  {
    Point point(-5.0f, 2.0f, 1.0f);
    Plane plane(glm::vec3(0.0f, 1.0f, 0.0f), 0.0f);
    REQUIRE(point_vs_plane(point, plane) == true);
  }

  SECTION("point on plane")
  {
    Point point(2.0f, 0.0f, -1.0f);
    Plane plane(glm::vec3(0.0f, 1.0f, 0.0f), 0.0f);
    REQUIRE(point_vs_plane(point, plane) == true);
  }

  SECTION("point behind plane")
  {
    Point point(1.5f, -2.0f, 6.0f);
    Plane plane(glm::vec3(0.0f, 1.0f, 0.0f), 0.0f);
    REQUIRE(point_vs_plane(point, plane) == false);
  }

  SECTION("point in front")
  {
    Point point(0.0f, 0.0f, -5.0f);
    Plane plane(glm::vec3(0.0f, 0.0f, 1.0f), 1000.0f);
    CHECK(point_vs_plane(point, plane) == true);
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

    glm::vec3 point(-1.0f);

    REQUIRE(plane_vs_plane_vs_plane(p0, p1, p2, point) == true);
    REQUIRE(point == glm::vec3(0.0f));
  }

  SECTION("no intersection")
  {
    Plane p0(glm::vec3(0.0f, 1.0f, 0.0f), 2.0f);
    Plane p1(glm::vec3(0.0f, 1.0f, 0.0f), 1.0f);
    Plane p2(glm::vec3(0.0f, 1.0f, 0.0f), 0.0f);
    glm::vec3 point;
    REQUIRE(plane_vs_plane_vs_plane(p0, p1, p2, point) == false);
  }
}

TEST_CASE("AABB vs Frustum")
{
  // camera looks down the negative z-axis
  float near = 1.0f;
  float far = 1000.0f;
  float aspect_ratio = 16.0f / 9.0f;
  float fov = 45.0f;
  glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
  glm::mat4 proj = glm::perspective(glm::radians(fov), aspect_ratio, near, far);
  glm::mat4 view_proj = proj * view;

  Frustum frustum(view_proj);

  SECTION("frustum creation")
  {
    REQUIRE(equal(frustum.near().normal, glm::vec3(0.0f, 0.0f, -1.0f)) == true);
    REQUIRE(equal(frustum.far().normal, glm::vec3(0.0f, 0.0f, 1.0f)) == true);
  }

  SECTION("aabb in front of camera")
  {
    glm::vec3 center = glm::vec3(0.0f, 0.0f, -5.0f);
    AABB bb = AABB::from_center_and_size(center, glm::vec3(1.0f));

#if 0
    auto vertices = frustum.vertices();
    AABB frustum_bb = AABB::from_points(std::span(vertices));


    std::cout << "vertices:\n";
    for (auto& v : vertices) std::cout << v << std::endl;

    std::cout << "frustum_bb:\n";
    std::cout << frustum_bb << std::endl;

    std::cout << "bb:\n";
    std::cout << bb << std::endl;

    CHECK(frustum_bb.contains(bb) == true);
#endif

    CHECK(point_vs_plane(center, frustum.near()) == true);
    CHECK(point_vs_plane(center, frustum.far()) == true);

    REQUIRE(aabb_vs_frustum(bb, frustum) == true);
  }

  SECTION("aabb behind camera")
  {
    glm::vec3 center = glm::vec3(0.0f, 0.0f, 5.0f);
    AABB bb = AABB::from_center_and_size(center, glm::vec3(1.0f));

#if 0
  auto vertices = frustum.vertices();
    AABB frustum_bb = AABB::from_points(std::span(vertices));

    std::cout << "bb:\n";
    std::cout << bb << std::endl;

    std::cout << "frustum:\n";
    std::cout << frustum << std::endl;

    std::cout << "vertices:\n";
    for (auto& v : vertices) std::cout << v << std::endl;

    std::cout << "frustum_bb:\n";
    std::cout << frustum_bb << std::endl;
    CHECK(frustum_bb.contains(bb) == false);
#endif

    CHECK(point_vs_plane(center, frustum.near()) == false);
    CHECK(point_vs_plane(center, frustum.far()) == true);

    REQUIRE(aabb_vs_frustum(bb, frustum) == false);
  }
}