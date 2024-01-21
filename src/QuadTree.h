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
};

inline std::ostream& operator<<(std::ostream& os, const Bounds& b) { return os << b.min << ", " << b.max; }

enum Direction : size_t { NORTH = 0, SOUTH = 1, EAST = 2, WEST = 3, NUM = 4 };

struct Node : public Bounds {
  Node(const glm::vec2& min_, const glm::vec2& max_) : Bounds(min_, max_) {}
  bool is_leaf{true};
  std::array<std::unique_ptr<Node>, 4> children = {nullptr};
};

class QuadTree
{
 public:
  QuadTree(const glm::vec2& min, const glm::vec2& max);
  void insert(const glm::vec2& point);
  std::vector<Node*> get_children() ;
  Node* get_root() const { return m_root.get(); }

 private:
  const float MIN_NODE_SIZE{0.25f};
  std::unique_ptr<Node> m_root{nullptr};
  void split_four_ways(std::unique_ptr<Node>& node);
  void insert(std::unique_ptr<Node>& child, const glm::vec2& point);
  void collect(std::unique_ptr<Node>& node, std::vector<Node*>& children);
};
