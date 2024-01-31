#pragma once

#include <array>
#include <glm/glm.hpp>
#include <memory>
#include <vector>

struct Node {
  glm::vec2 min{}, max{};
  bool is_leaf{true};
  unsigned depth{0};
  Node* parent{nullptr};
  std::array<std::unique_ptr<Node>, 4> children;

  Node(const glm::vec2& min_, const glm::vec2& max_, unsigned depth_ = 0)
      : children{nullptr}, min(min_), max(max_), depth(depth_)
  {
  }

  inline glm::vec2 size() const { return max - min; }

  inline glm::vec2 center() const { return min + size() / 2.0f; }

  inline bool contains(const glm::vec2& point) const
  {
    return glm::all(glm::lessThanEqual(min, point)) && glm::all(glm::lessThanEqual(point, max));
  }

  inline Node* SW() const { return children[0].get(); }
  inline Node* NW() const { return children[1].get(); }
  inline Node* NE() const { return children[2].get(); }
  inline Node* SE() const { return children[3].get(); }

  std::vector<Node*> neighbours() const;
};

class QuadTree
{
 public:
  QuadTree(const glm::vec2& min, const glm::vec2& max, unsigned m_max_depth);
  void insert(const glm::vec2& point);
  std::vector<Node*> children();
  Node* root() const { return m_root.get(); }

  template <typename Visitor>
  void visit(Visitor visitor) const
  {
    assert(m_root != nullptr);
    visit(m_root, visitor);
  }

 private:
  const unsigned m_max_depth;
  std::unique_ptr<Node> m_root{nullptr};

  void split(std::unique_ptr<Node>& node);

  void insert(std::unique_ptr<Node>& child, const glm::vec2& point);

  template <typename Visitor>
  void visit(const std::unique_ptr<Node>& node, Visitor visitor) const
  {
    if (node && visitor(node.get()) && !node->is_leaf) {
      for (const auto& child : node->children) {
        visit(child, visitor);
      }
    }
  }
};
