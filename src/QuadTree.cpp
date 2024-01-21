#include "QuadTree.h"

QuadTree::QuadTree(const glm::vec2& min, const glm::vec2& max) { m_root = std::make_unique<Node>(min, max); }

void QuadTree::build_around_point(const glm::vec2& point) { insert(m_root, point); }

std::vector<Node*> QuadTree::get_children() const { return {get_root()}; }

void QuadTree::insert(std::unique_ptr<Node>& child, const glm::vec2& position)
{
  float distance_to_child = glm::length(child->center() - position);

  if (distance_to_child < child->size().x && child->size().x > MIN_NODE_SIZE) {

  }
}

