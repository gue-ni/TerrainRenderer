#include "QuadTree.h"

QuadTree::QuadTree(const glm::vec2& min, const glm::vec2& max, float min_node_size, unsigned max_depth)
    : m_root(std::make_unique<Node>(min, max)), MIN_NODE_SIZE(min_node_size), MAX_DEPTH(max_depth)
{
  Bounds bounds(min, max);
  assert(bounds.size().x > min_node_size);
}

void QuadTree::insert(const glm::vec2& point)
{
  assert(m_root->contains(point));
  insert(m_root, point);
}

std::vector<Node*> QuadTree::get_children()
{
  std::vector<Node*> children;
  collect(m_root, children);
  return children;
}

void QuadTree::insert(std::unique_ptr<Node>& node, const glm::vec2& point)
{
  float size = node->size().x;
  float distance = glm::distance(node->center(), point);

  if (distance < size && node->depth < MAX_DEPTH) {
    split(node);
    for (auto& child : node->children) insert(child, point);
  }
}

void QuadTree::split(std::unique_ptr<Node>& node)
{
  auto new_depth = node->depth + 1;
  auto min = node->min, max = node->max, midpoint = node->center();
  node->children[0] = std::make_unique<Node>(min, midpoint, new_depth);  // lower left

  node->children[1] =
      std::make_unique<Node>(glm::vec2(min.x, midpoint.y), glm::vec2(midpoint.x, max.y), new_depth);  // upper left

  node->children[2] = std::make_unique<Node>(midpoint, max, new_depth);  // upper right

  node->children[3] =
      std::make_unique<Node>(glm::vec2{midpoint.x, min.y}, glm::vec2{max.x, midpoint.y}, new_depth);  // lower right

  node->is_leaf = false;
}

void QuadTree::collect(std::unique_ptr<Node>& node, std::vector<Node*>& children)
{
  if (node->is_leaf) {
    children.push_back(node.get());
  } else {
    for (auto& child : node->children) collect(child, children);
  }
}
