#include "QuadTree.h"

QuadTree::QuadTree(const glm::vec2& min, const glm::vec2& max, unsigned m_max_depth)
    : m_root(std::make_unique<Node>(min, max)), m_max_depth(m_max_depth)
{
}

void QuadTree::insert(const glm::vec2& point)
{
  assert(m_root->contains(point));
  insert(m_root, point);
}

std::vector<Node*> QuadTree::children()
{
  std::vector<Node*> children;

  visit([&children](Node* node) {
    children.push_back(node);
    return true;
  });

  return children;
}

void QuadTree::insert(std::unique_ptr<Node>& node, const glm::vec2& point)
{
  float width = node->size().x;
  float distance = glm::distance(node->center(), point);
  float factor = .75f;

  if (distance * factor < width && node->depth < m_max_depth) {
    split(node);
    for (auto& child : node->children) insert(child, point);
  }
}

void QuadTree::split(std::unique_ptr<Node>& node)
{
  auto child_depth = node->depth + 1;
  auto min = node->min, max = node->max, midpoint = node->center();

  node->is_leaf = false;
  // lower left
  node->children[Dir::SW] = std::make_unique<Node>(min, midpoint, child_depth);
  // upper left
  node->children[Dir::NW] =
      std::make_unique<Node>(glm::vec2(min.x, midpoint.y), glm::vec2(midpoint.x, max.y), child_depth);
  // upper right
  node->children[Dir::NE] = std::make_unique<Node>(midpoint, max, child_depth);
  // lower right
  node->children[Dir::SE] =
      std::make_unique<Node>(glm::vec2{midpoint.x, min.y}, glm::vec2{max.x, midpoint.y}, child_depth);

  for (auto& child : node->children) child->parent = node.get();
}

std::vector<Node*> Node::neighbours() const { return {}; }
