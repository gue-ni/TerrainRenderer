#include <catch2/catch_test_macros.hpp>
#include <iostream>

#include "Common.h"
#include "QuadTree.h"
#include "TileUtils.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/io.hpp>

void print(const std::vector<Node*> nodes)
{
  for (const Node* node : nodes) {
    std::cout << node->min << node->max << " " << node->depth << std::endl;
  }
}

void print(QuadTree& quad_tree)
{
  auto nodes = quad_tree.nodes();
  auto leaves = quad_tree.leaves();

  std::cout << "QuadTree:\n";
  std::cout << quad_tree.root()->min << quad_tree.root()->max << ", " << quad_tree.max_depth() << std::endl;
  std::cout << "Nodes:\n";
  print(nodes);
  std::cout << "Leaves:\n";
  print(leaves);
}

TEST_CASE("Create QuadTree")
{
  const TileId root_tile = TileId(0U, 0U, 0U);
  const auto bounds = Bounds<glm::vec2>(glm::vec2(0.0f), glm::vec2(100.0f));

  SECTION("depth == 1")
  {
    QuadTree quad_tree(bounds.center(), bounds.min, bounds.max, 1, root_tile);

    auto nodes = quad_tree.nodes();
    auto leaves = quad_tree.leaves();

    // print(quad_tree);

    REQUIRE(leaves.size() == 4);
    REQUIRE(nodes.size() == 5);
  }

  SECTION("depth == 2")
  {
    QuadTree quad_tree(bounds.center(), bounds.min, bounds.max, 2, root_tile);

    auto nodes = quad_tree.nodes();
    auto leaves = quad_tree.leaves();

    // print(quad_tree);

    // REQUIRE(leaves.size() == (3 + 4));
    // REQUIRE(nodes.size() == (1 + 4 + 4));
  }
}
