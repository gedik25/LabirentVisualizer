#pragma once

#include "ISolver.hpp"
#include <functional>
#include <queue>

namespace maze {

/**
 * Jump Point Search (JPS) pathfinding algorithm.
 * An optimization of A* that reduces the number of nodes
 * expanded by "jumping" along straight lines until a
 * forced neighbor (wall corner) is found.
 *
 * On block-wall grids JPS operates in cardinal directions only.
 * For each direction it scans forward skipping empty cells.
 * A cell is a "jump point" when:
 *   - It is the goal, OR
 *   - A wall beside it opens up a new forced direction.
 *
 * Characteristics:
 * - Much faster than A* on open maps
 * - Same optimal guarantees as A*
 * - Cardinal-only variant (no diagonals)
 */
class JPSSolver : public SolverBase {
public:
  explicit JPSSolver(std::shared_ptr<Grid> grid);

  void init(const Position &start, const Position &end) override;
  bool step() override;
  bool solve() override;
  void reset() override;
  std::string getName() const override { return "JPS"; }

private:
  struct Node {
    Position pos;
    float fScore;
    bool operator>(const Node &other) const { return fScore > other.fScore; }
  };

  std::priority_queue<Node, std::vector<Node>, std::greater<Node>> m_openSet;
  std::vector<std::vector<float>> m_gScore;
  std::vector<std::vector<Position>> m_parent;
  std::vector<std::vector<bool>> m_inOpenSet;
  std::vector<std::vector<bool>> m_closedSet;

  void initDataStructures();
  float heuristic(const Position &a, const Position &b) const;

  // Jump in a given direction. Returns the jump point or {-1,-1} if none.
  Position jump(const Position &pos, int dx, int dy) const;

  // Check if a cell is walkable (valid and not AllWalls)
  bool isWalkable(int x, int y) const;

  // Get successors (jump points) from current node
  std::vector<Position> getSuccessors(const Position &current) const;
};

} // namespace maze
