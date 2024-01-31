#include "QuadTree.h"

QuadTree::QuadTree(const glm::vec2& min, const glm::vec2& max, unsigned m_max_depth)
    : m_root(std::make_unique<Node>(min, max, 0, nullptr)), m_max_depth(m_max_depth)
{
}

void QuadTree::insert(const glm::vec2& point)
{
  assert(m_root->contains(point));
  insert(m_root, point);
}

std::vector<Node*> QuadTree::nodes()
{
  std::vector<Node*> nodes;
  visit([&nodes](Node* node) { nodes.push_back(node); });
  return nodes;
}

void QuadTree::insert(std::unique_ptr<Node>& node, const glm::vec2& point)
{
  float width = node->size().x;
  float distance = glm::distance(node->center(), point);
  float factor = .75f;

  if ((distance * factor) < width && node->depth < m_max_depth) {
    node->split();
    for (auto& child : node->children) {
      insert(child, point);
    }
  }
}

std::vector<Node*> Node::neighbours() const { return {}; }

void Node::split()
{
  auto child_depth = depth + 1;
  auto middle = center();

  is_leaf = false;
  children[Dir::NE] = std::make_unique<Node>(middle, max, child_depth, this);
  children[Dir::NW] = std::make_unique<Node>(glm::vec2(min.x, middle.y), glm::vec2(middle.x, max.y), child_depth, this);
  children[Dir::SW] = std::make_unique<Node>(min, middle, child_depth, this);
  children[Dir::SE] = std::make_unique<Node>(glm::vec2{middle.x, min.y}, glm::vec2{max.x, middle.y}, child_depth, this);
}
