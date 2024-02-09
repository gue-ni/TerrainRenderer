#include "QuadTree.h"

QuadTree::QuadTree(const glm::vec2& point, const glm::vec2& min, const glm::vec2& max, unsigned max_depth)
    : m_root(std::make_unique<Node>(min, max, 0, nullptr)), m_max_depth(max_depth)
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

std::vector<Node*> QuadTree::leaves()
{
  std::vector<Node*> leaves;
  visit([&leaves](Node* node) {
    if (node->is_leaf) {
      leaves.push_back(node);
    }
  });
  return leaves;
}

void QuadTree::insert(std::unique_ptr<Node>& node, const glm::vec2& point)
{
  auto split_heuristic = [](const glm::vec2& p, Node* n, unsigned max_depth) {
    if (max_depth <= n->depth) {
      return false;
    }

    float width = n->size().x;
    float distance = glm::distance(n->center(), p);
    float factor = 0.75f;
    return (distance * factor) < width;
  };

  if (split_heuristic(point, node.get(), m_max_depth)) {
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
  children[NE] = std::make_unique<Node>(middle, max, child_depth, this);
  children[NW] = std::make_unique<Node>(glm::vec2(min.x, middle.y), glm::vec2(middle.x, max.y), child_depth, this);
  children[SW] = std::make_unique<Node>(min, middle, child_depth, this);
  children[SE] = std::make_unique<Node>(glm::vec2{middle.x, min.y}, glm::vec2{max.x, middle.y}, child_depth, this);
}
