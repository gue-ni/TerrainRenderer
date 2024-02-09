#pragma once

#include <array>
#include <glm/glm.hpp>
#include <memory>
#include <vector>

struct Node {
  enum : std::size_t { NE = 0, NW, SE, SW };
  glm::vec2 min, max;
  bool is_leaf;
  unsigned depth;
  Node* parent;
  std::array<std::unique_ptr<Node>, 4> children;

  Node(const glm::vec2& min_, const glm::vec2& max_, unsigned depth_, Node* parent_ = nullptr)
      : children{nullptr}, min(min_), max(max_), depth(depth_), parent(parent_), is_leaf(true)
  {
  }

  inline glm::vec2 size() const { return max - min; }

  inline glm::vec2 center() const { return min + size() / 2.0f; }

  inline bool contains(const glm::vec2& point) const
  {
    return glm::all(glm::lessThanEqual(min, point)) && glm::all(glm::lessThanEqual(point, max));
  }

  std::vector<Node*> neighbours() const;

  void split();
};

class QuadTree
{
 public:
  QuadTree(const glm::vec2& point, const glm::vec2& min, const glm::vec2& max, unsigned m_max_depth);

  std::vector<Node*> nodes();

  std::vector<Node*> leaves();

  Node* root() const { return m_root.get(); }

  unsigned max_depth() const { return m_max_depth; }

  template <typename Visitor>
  void visit(Visitor visitor) const
  {
    assert(m_root != nullptr);
    visit(m_root, visitor);
  }

 private:
  const unsigned m_max_depth;
  std::unique_ptr<Node> m_root{nullptr};

  void insert(std::unique_ptr<Node>& child, const glm::vec2& point);

  template <typename Visitor>
  void visit(const std::unique_ptr<Node>& node, Visitor visitor) const
  {
    assert(node != nullptr);

    visitor(node.get());

    if (!node->is_leaf) {
      for (const auto& child : node->children) {
        visit(child, visitor);
      }
    }
  }
};
