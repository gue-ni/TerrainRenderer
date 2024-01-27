#pragma once

#include <array>
#include <glm/glm.hpp>
#include <glm/gtx/io.hpp>
#include <iostream>
#include <memory>
#include <vector>

struct Bounds {
  glm::vec2 min{}, max{};
  Bounds(const glm::vec2& min_, const glm::vec2& max_) : min{min_}, max{max_} {}
  glm::vec2 size() const { return max - min; }
  glm::vec2 half_size() const { return size() / 2.0f; }
  glm::vec2 center() const { return min + half_size(); }
  bool contains(const glm::vec2& point)
  {
    return glm::all(glm::lessThanEqual(min, point)) && glm::all(glm::lessThanEqual(point, max));
  }
};

inline std::ostream& operator<<(std::ostream& os, const Bounds& b) { return os << b.min << ", " << b.max; }

struct Node : public Bounds {
  Node(const glm::vec2& min_, const glm::vec2& max_, unsigned depth_ = 0) : Bounds(min_, max_), depth(depth_) {}
  bool is_leaf{true};
  unsigned depth{0};
  std::array<std::unique_ptr<Node>, 4> children;
  Node* parent{nullptr};
};

class QuadTree
{
 public:
  QuadTree(const glm::vec2& min, const glm::vec2& max, unsigned max_depth);
  void insert(const glm::vec2& point);
  std::vector<Node*> get_children();
  Node* root() const { return m_root.get(); }

  template <typename Func>
  void traverse(Func func) const
  {
    assert(m_root != nullptr);
    traverse(m_root, func);
  }

 private:
  const unsigned max_depth;
  std::unique_ptr<Node> m_root{nullptr};
  void split(std::unique_ptr<Node>& node);
  void insert(std::unique_ptr<Node>& child, const glm::vec2& point);
  void collect(std::unique_ptr<Node>& node, std::vector<Node*>& children);

  template <typename Func>
  void traverse(const std::unique_ptr<Node>& node, Func func) const
  {
    if (node && func(node.get()) && !node->is_leaf) {
      for (const auto& child : node->children) {
        traverse(child, func);
      }
    }
  }
};
