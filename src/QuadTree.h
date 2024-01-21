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
  bool contains_point(const glm::vec2& point)
  {
    return (min.x <= point.x && point.x <= max.x) && (min.y <= point.y && point.y <= max.y);
  }
};

inline std::ostream& operator<<(std::ostream& os, const Bounds& b) { return os << b.min << ", " << b.max; }

struct Node : public Bounds {
  Node(const glm::vec2& min_, const glm::vec2& max_, unsigned lod_ = 0) : Bounds(min_, max_), lod(lod_) {}
  bool is_leaf{true};
  unsigned lod = 0;
  std::array<std::unique_ptr<Node>, 4> children;
};

class QuadTree
{
 public:
  QuadTree(const glm::vec2& min, const glm::vec2& max, float min_node_size);
  void insert(const glm::vec2& point);
  std::vector<Node*> get_children();

 private:
  const float MIN_NODE_SIZE;
  std::unique_ptr<Node> m_root{nullptr};
  void split(std::unique_ptr<Node>& node);
  void insert(std::unique_ptr<Node>& child, const glm::vec2& point);
  void collect(std::unique_ptr<Node>& node, std::vector<Node*>& children);
};
