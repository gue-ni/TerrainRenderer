#pragma once

#include <array>
#include <glm/glm.hpp>
#include <glm/gtx/io.hpp>
#include <iostream>
#include <memory>
#include <vector>

struct Bounds {
  glm::vec2 min{}, max{};
  Bounds(const glm::vec2& min_, const glm::vec2& max_) : min(min_), max(max_) {}
  glm::vec2 size() const { return max - min; }
  glm::vec2 half_size() const { return size() / 2.0f; }
  glm::vec2 center() const { return min + half_size(); }
  std::vector<Bounds> split_four_ways() const
  {
    auto midpoint = center();
    return {
        Bounds(min, midpoint),
        Bounds(midpoint, max),
        Bounds(glm::vec2(min.x, min.y), glm::vec2(max.x, max.y)),
        Bounds(glm::vec2(min.x, min.y), glm::vec2(max.x, max.y)),
    };
  }
};

inline std::ostream& operator<<(std::ostream& os, const Bounds& b) { return os << b.min << ", " << b.max; }

enum Direction : size_t { NORTH = 0, SOUTH = 1, EAST = 2, WEST = 3, NUM = 4 };

struct Node : public Bounds {
  Node(const glm::vec2& min_, const glm::vec2& max_) : Bounds(min_, max_) {}
  std::array<std::unique_ptr<Node>, 4> children;
};

class QuadTree
{
 public:
  QuadTree(const glm::vec2& min, const glm::vec2& max);
  void build_around_point(const glm::vec2& point);
  std::vector<Node*> get_children() const;
  Node* get_root() const { return m_root.get(); }

 private:
  std::unique_ptr<Node> m_root{nullptr};
  void insert(std::unique_ptr<Node>& child, const glm::vec2& position);
  const float MIN_NODE_SIZE{.25f};
};
