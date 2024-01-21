#include "QuadTree.h"

QuadTree::QuadTree(const glm::vec2& min, const glm::vec2& max) { m_root = std::make_unique<Node>(min, max); }

void QuadTree::insert(const glm::vec2& point) { insert(m_root, point); }

std::vector<Node*> QuadTree::get_children()
{
  std::vector<Node*> children;
  collect(m_root, children);
  return children;
}

void QuadTree::insert(std::unique_ptr<Node>& node, const glm::vec2& point)
{
  float distance_to_child = glm::distance(node->center(), point);

  if (distance_to_child < node->size().x && node->size().x >= MIN_NODE_SIZE) {
    split_four_ways(node);

    for (std::unique_ptr<Node>& child : node->children) {
      insert(child, point);
    }
  }
}

void QuadTree::split_four_ways(std::unique_ptr<Node>& node)
{
  auto min = node->min, max = node->max;
  auto midpoint = node->center();
  node->children[0] = std::make_unique<Node>(min, midpoint);
  node->children[1] = std::make_unique<Node>(min, max);
  node->children[2] = std::make_unique<Node>(midpoint, max);
  node->children[3] = std::make_unique<Node>(min, max);
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
