#include "JPSSolver.hpp"
#include <chrono>
#include <cmath>
#include <limits>

namespace maze {

JPSSolver::JPSSolver(std::shared_ptr<Grid> grid)
    : SolverBase(std::move(grid)) {}

void JPSSolver::init(const Position &start, const Position &end) {
  reset();

  m_start = start;
  m_end = end;
  m_current = start;

  initDataStructures();

  m_gScore[start.y][start.x] = 0;
  float h = heuristic(start, end);

  m_openSet.push({start, h});
  m_inOpenSet[start.y][start.x] = true;
  m_grid->setFlag(start, CellFlags::Queued);
  m_stats.nodesInQueue = 1;
}

void JPSSolver::initDataStructures() {
  int width = m_grid->getWidth();
  int height = m_grid->getHeight();

  constexpr float INF = std::numeric_limits<float>::infinity();

  m_gScore.assign(height, std::vector<float>(width, INF));
  m_parent.assign(height, std::vector<Position>(width, {-1, -1}));
  m_inOpenSet.assign(height, std::vector<bool>(width, false));
  m_closedSet.assign(height, std::vector<bool>(width, false));

  while (!m_openSet.empty()) {
    m_openSet.pop();
  }
}

float JPSSolver::heuristic(const Position &a, const Position &b) const {
  return static_cast<float>(std::abs(a.x - b.x) + std::abs(a.y - b.y));
}

bool JPSSolver::isWalkable(int x, int y) const {
  if (!m_grid->isValid(x, y))
    return false;
  // A cell with all wall flags set is a block-wall
  return !m_grid->hasFlag(x, y, CellFlags::AllWalls);
}

Position JPSSolver::jump(const Position &pos, int dx, int dy) const {
  int nx = pos.x + dx;
  int ny = pos.y + dy;

  // Out of bounds or hit a wall
  if (!isWalkable(nx, ny)) {
    return {-1, -1};
  }

  Position next{nx, ny};

  // Reached the goal — this is a jump point
  if (next == m_end) {
    return next;
  }

  // Cardinal movement only (dx or dy is 0)
  // Check for forced neighbors
  if (dx != 0) {
    // Moving horizontally: check vertical forced neighbors
    // If the cell above/below is blocked but the diagonal-adjacent cell is
    // open, forced
    if ((!isWalkable(nx, ny - 1)) && isWalkable(nx + dx, ny - 1)) {
      return next;
    }
    if ((!isWalkable(nx, ny + 1)) && isWalkable(nx + dx, ny + 1)) {
      return next;
    }
    // For horizontal movement, also check if perpendicular forced neighbors
    // exist
    if ((!isWalkable(nx, ny - 1)) && isWalkable(nx, ny - 1 + 0)) {
      // This is already handled above — skip.
    }
  }
  if (dy != 0) {
    // Moving vertically: check horizontal forced neighbors
    if ((!isWalkable(nx - 1, ny)) && isWalkable(nx - 1, ny + dy)) {
      return next;
    }
    if ((!isWalkable(nx + 1, ny)) && isWalkable(nx + 1, ny + dy)) {
      return next;
    }
  }

  // Continue jumping in the same direction
  return jump(next, dx, dy);
}

std::vector<Position> JPSSolver::getSuccessors(const Position &current) const {
  std::vector<Position> successors;

  // Four cardinal directions
  static const int dirs[4][2] = {{0, -1}, {1, 0}, {0, 1}, {-1, 0}};

  for (const auto &d : dirs) {
    Position jp = jump(current, d[0], d[1]);
    if (jp.x != -1) {
      successors.push_back(jp);
    }
  }

  return successors;
}

bool JPSSolver::step() {
  if (m_finished) {
    return false;
  }

  auto startTime = std::chrono::high_resolution_clock::now();

  if (m_openSet.empty()) {
    m_finished = true;
    m_pathFound = false;
    return false;
  }

  m_grid->clearFlag(m_current, CellFlags::Current);

  Node currentNode = m_openSet.top();
  m_openSet.pop();
  m_current = currentNode.pos;

  if (m_closedSet[m_current.y][m_current.x]) {
    auto endTime = std::chrono::high_resolution_clock::now();
    m_stats.elapsedTime +=
        std::chrono::duration<float, std::milli>(endTime - startTime).count();
    return true;
  }

  m_closedSet[m_current.y][m_current.x] = true;
  m_inOpenSet[m_current.y][m_current.x] = false;
  m_grid->clearFlag(m_current, CellFlags::Queued);
  m_grid->setFlag(m_current, CellFlags::Visited);
  m_grid->setFlag(m_current, CellFlags::Current);
  m_visited.push_back(m_current);
  ++m_stats.nodesVisited;

  m_stats.nodesInQueue = m_openSet.size();

  if (m_current == m_end) {
    m_finished = true;
    m_pathFound = true;
    reconstructPath(m_parent);
    m_grid->clearFlag(m_current, CellFlags::Current);

    auto endTime = std::chrono::high_resolution_clock::now();
    m_stats.elapsedTime +=
        std::chrono::duration<float, std::milli>(endTime - startTime).count();
    return false;
  }

  // Get jump point successors
  std::vector<Position> successors = getSuccessors(m_current);

  for (const Position &jp : successors) {
    if (m_closedSet[jp.y][jp.x])
      continue;

    // Cost is Manhattan distance between current and jump point
    float dist = static_cast<float>(std::abs(jp.x - m_current.x) +
                                    std::abs(jp.y - m_current.y));

    // For terrain support: accumulate terrain costs along the jump path
    float terrainCost = 0.0f;
    int dx = (jp.x > m_current.x) ? 1 : (jp.x < m_current.x) ? -1 : 0;
    int dy = (jp.y > m_current.y) ? 1 : (jp.y < m_current.y) ? -1 : 0;
    Position walk = m_current;
    for (int i = 0; i < static_cast<int>(dist); ++i) {
      Position next{walk.x + dx, walk.y + dy};
      terrainCost += m_grid->getMovementCost(walk, next);
      walk = next;
    }

    float tentativeG = m_gScore[m_current.y][m_current.x] + terrainCost;

    if (tentativeG < m_gScore[jp.y][jp.x]) {
      m_parent[jp.y][jp.x] = m_current;
      m_gScore[jp.y][jp.x] = tentativeG;
      float fScore = tentativeG + heuristic(jp, m_end);

      if (!m_inOpenSet[jp.y][jp.x]) {
        m_openSet.push({jp, fScore});
        m_inOpenSet[jp.y][jp.x] = true;
        m_grid->setFlag(jp, CellFlags::Queued);
      }
    }

    // Mark intermediate cells along jump as visited for visualization
    walk = m_current;
    for (int i = 0; i < static_cast<int>(dist) - 1; ++i) {
      walk = {walk.x + dx, walk.y + dy};
      if (!m_closedSet[walk.y][walk.x]) {
        m_grid->setFlag(walk, CellFlags::Visited);
        m_visited.push_back(walk);
      }
    }
  }

  auto endTime = std::chrono::high_resolution_clock::now();
  m_stats.elapsedTime +=
      std::chrono::duration<float, std::milli>(endTime - startTime).count();

  return true;
}

bool JPSSolver::solve() {
  while (step()) {
  }
  return m_pathFound;
}

void JPSSolver::reset() {
  SolverBase::reset();

  while (!m_openSet.empty()) {
    m_openSet.pop();
  }
  m_gScore.clear();
  m_parent.clear();
  m_inOpenSet.clear();
  m_closedSet.clear();
}

} // namespace maze
