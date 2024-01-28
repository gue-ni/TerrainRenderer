#pragma once

#include <array>
#include <glm/glm.hpp>
#include <memory>
#include <vector>

struct Node  {
  glm::vec2 min{}, max{};
  bool is_leaf{true};
  unsigned depth{0};
  Node* parent{nullptr};
  std::array<std::unique_ptr<Node>, 4> children;

  Node(const glm::vec2& min_, const glm::vec2& max_, unsigned depth_ = 0) : min(min_), max(max_), depth(depth_) {}
  glm::vec2 size() const { return max - min; }
  glm::vec2 center() const { return min + size() / 2.0f; }
  bool contains(const glm::vec2& point)
  {
    return glm::all(glm::lessThanEqual(min, point)) && glm::all(glm::lessThanEqual(point, max));
  }
};

class QuadTree
{
 public:
  QuadTree(const glm::vec2& min, const glm::vec2& max, unsigned max_depth);
  void insert(const glm::vec2& point);
  std::vector<Node*> children();
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
