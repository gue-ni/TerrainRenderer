#include "QuadTree.h"

QuadTree::QuadTree(const glm::vec2& min, const glm::vec2& max, unsigned max_depth)
    : m_root(std::make_unique<Node>(min, max)), max_depth(max_depth)
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
  collect(m_root, children);
  return children;
}

void QuadTree::insert(std::unique_ptr<Node>& node, const glm::vec2& point)
{
  float size = node->size().x;
  float distance = glm::distance(node->center(), point);

  if (distance < size && node->depth < max_depth) {
    split(node);
    for (auto& child : node->children) insert(child, point);
  }
}

void QuadTree::split(std::unique_ptr<Node>& node)
{
  auto child_depth = node->depth + 1;
  auto min = node->min, max = node->max, midpoint = node->center();

  node->is_leaf = false;
  node->children[0] = std::make_unique<Node>(min, midpoint, child_depth);
  node->children[1] = std::make_unique<Node>(glm::vec2(min.x, midpoint.y), glm::vec2(midpoint.x, max.y), child_depth);
  node->children[2] = std::make_unique<Node>(midpoint, max, child_depth);
  node->children[3] = std::make_unique<Node>(glm::vec2{midpoint.x, min.y}, glm::vec2{max.x, midpoint.y}, child_depth);

  for (auto& child : node->children) child->parent = node.get();
}

void QuadTree::collect(std::unique_ptr<Node>& node, std::vector<Node*>& children)
{
  if (node->is_leaf) {
    children.push_back(node.get());
  } else {
    for (auto& child : node->children) collect(child, children);
  }
}
