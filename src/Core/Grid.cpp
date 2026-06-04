#include "Grid.hpp"
#include <fstream>

namespace maze {

Grid::Grid(int width, int height)
    : m_width(width), m_height(height),
      m_cells(static_cast<size_t>(width * height), CellFlags::AllWalls),
      m_terrain(static_cast<size_t>(width * height),
                static_cast<uint8_t>(TerrainType::Normal)),
      m_start{0, 0}, m_end{width - 1, height - 1} {
  if (width <= 0 || height <= 0) {
    throw std::invalid_argument("Grid dimensions must be positive");
  }
}

bool Grid::isValid(int x, int y) const {
  return x >= 0 && x < m_width && y >= 0 && y < m_height;
}

bool Grid::isValid(const Position &pos) const { return isValid(pos.x, pos.y); }

uint8_t Grid::getCell(int x, int y) const {
  if (!isValid(x, y)) {
    throw std::out_of_range("Cell position out of bounds");
  }
  return m_cells[index(x, y)];
}

uint8_t Grid::getCell(const Position &pos) const {
  return getCell(pos.x, pos.y);
}

void Grid::setCell(int x, int y, uint8_t value) {
  if (!isValid(x, y)) {
    throw std::out_of_range("Cell position out of bounds");
  }
  m_cells[index(x, y)] = value;
}

void Grid::setCell(const Position &pos, uint8_t value) {
  setCell(pos.x, pos.y, value);
}

bool Grid::hasFlag(int x, int y, uint8_t flag) const {
  return maze::hasFlag(getCell(x, y), flag);
}

bool Grid::hasFlag(const Position &pos, uint8_t flag) const {
  return hasFlag(pos.x, pos.y, flag);
}

void Grid::setFlag(int x, int y, uint8_t flag) {
  if (!isValid(x, y))
    return;
  maze::setFlag(m_cells[index(x, y)], flag);
}

void Grid::setFlag(const Position &pos, uint8_t flag) {
  setFlag(pos.x, pos.y, flag);
}

void Grid::clearFlag(int x, int y, uint8_t flag) {
  if (!isValid(x, y))
    return;
  maze::clearFlag(m_cells[index(x, y)], flag);
}

void Grid::clearFlag(const Position &pos, uint8_t flag) {
  clearFlag(pos.x, pos.y, flag);
}

bool Grid::hasWall(int x, int y, Direction dir) const {
  return hasFlag(x, y, wallFlagForDirection(dir));
}

bool Grid::hasWall(const Position &pos, Direction dir) const {
  return hasWall(pos.x, pos.y, dir);
}

void Grid::setWall(int x, int y, Direction dir) {
  setFlag(x, y, wallFlagForDirection(dir));
}

void Grid::removeWall(int x, int y, Direction dir) {
  clearFlag(x, y, wallFlagForDirection(dir));
}

void Grid::removeWallBetween(const Position &from, const Position &to) {
  int dx = to.x - from.x;
  int dy = to.y - from.y;

  Direction dir;
  if (dx == 1 && dy == 0) {
    dir = Direction::East;
  } else if (dx == -1 && dy == 0) {
    dir = Direction::West;
  } else if (dx == 0 && dy == 1) {
    dir = Direction::South;
  } else if (dx == 0 && dy == -1) {
    dir = Direction::North;
  } else {
    return; // Not adjacent cells
  }

  // Remove wall from both sides
  removeWall(from.x, from.y, dir);
  removeWall(to.x, to.y, oppositeDirection(dir));
}

bool Grid::canMove(const Position &from, Direction dir) const {
  if (!isValid(from))
    return false;
  if (hasWall(from, dir))
    return false;

  Position neighbor = getNeighbor(from, dir);
  return isValid(neighbor);
}

bool Grid::canMove(const Position &from, const Position &to) const {
  int dx = to.x - from.x;
  int dy = to.y - from.y;

  // Must be adjacent
  if (std::abs(dx) + std::abs(dy) != 1)
    return false;

  Direction dir;
  if (dx == 1)
    dir = Direction::East;
  else if (dx == -1)
    dir = Direction::West;
  else if (dy == 1)
    dir = Direction::South;
  else
    dir = Direction::North;

  return canMove(from, dir);
}

Position Grid::getNeighbor(const Position &pos, Direction dir) const {
  Position offset = directionOffset(dir);
  return {pos.x + offset.x, pos.y + offset.y};
}

std::vector<Position> Grid::getNeighbors(const Position &pos) const {
  std::vector<Position> neighbors;
  neighbors.reserve(4);

  for (int d = 0; d < 4; ++d) {
    Direction dir = static_cast<Direction>(d);
    Position neighbor = getNeighbor(pos, dir);
    if (isValid(neighbor)) {
      neighbors.push_back(neighbor);
    }
  }

  return neighbors;
}

std::vector<Position> Grid::getAccessibleNeighbors(const Position &pos) const {
  std::vector<Position> neighbors;
  neighbors.reserve(4);

  for (int d = 0; d < 4; ++d) {
    Direction dir = static_cast<Direction>(d);
    if (canMove(pos, dir)) {
      neighbors.push_back(getNeighbor(pos, dir));
    }
  }

  return neighbors;
}

void Grid::reset() {
  std::fill(m_cells.begin(), m_cells.end(), CellFlags::AllWalls);
  // Don't reset terrain - it should be preserved or explicitly cleared
}

void Grid::clearSolverState() {
  constexpr uint8_t solverFlags = CellFlags::Visited | CellFlags::InPath |
                                  CellFlags::Current | CellFlags::Queued;
  for (auto &cell : m_cells) {
    cell &= ~solverFlags;
  }
}

// Terrain methods
TerrainType Grid::getTerrain(int x, int y) const {
  if (!isValid(x, y)) {
    return TerrainType::Normal;
  }
  return static_cast<TerrainType>(m_terrain[index(x, y)]);
}

TerrainType Grid::getTerrain(const Position &pos) const {
  return getTerrain(pos.x, pos.y);
}

void Grid::setTerrain(int x, int y, TerrainType type) {
  if (!isValid(x, y))
    return;
  m_terrain[index(x, y)] = static_cast<uint8_t>(type);
}

void Grid::setTerrain(const Position &pos, TerrainType type) {
  setTerrain(pos.x, pos.y, type);
}

float Grid::getMovementCost(const Position &from, const Position &to) const {
  // Average cost of both cells
  float fromCost = getMovementCost(from);
  float toCost = getMovementCost(to);
  return (fromCost + toCost) / 2.0f;
}

float Grid::getMovementCost(const Position &pos) const {
  return getTerrainCost(getTerrain(pos));
}

void Grid::generateTerrain(const TerrainDistribution &dist, unsigned int seed) {
  std::mt19937 rng(seed);
  std::uniform_real_distribution<float> uniform(0.0f, 1.0f);

  auto probs = dist.toArray();

  // Calculate cumulative probabilities
  std::array<float, TERRAIN_COUNT> cumulative;
  float sum = 0.0f;
  for (size_t i = 0; i < TERRAIN_COUNT; ++i) {
    sum += probs[i];
    cumulative[i] = sum;
  }

  // Normalize if needed
  if (sum > 0.0f) {
    for (auto &c : cumulative) {
      c /= sum;
    }
  }

  // Helper to pick terrain type
  auto pickTerrain = [&]() -> TerrainType {
    float r = uniform(rng);
    for (size_t i = 0; i < TERRAIN_COUNT; ++i) {
      if (r < cumulative[i]) {
        return static_cast<TerrainType>(i);
      }
    }
    return TerrainType::Normal;
  };

  // Generate terrain with clustering
  std::vector<bool> assigned(m_cells.size(), false);

  for (int y = 0; y < m_height; ++y) {
    for (int x = 0; x < m_width; ++x) {
      size_t idx = index(x, y);
      if (assigned[idx])
        continue;

      // Pick terrain type
      TerrainType type = pickTerrain();

      // Start a cluster
      std::vector<Position> cluster;
      cluster.push_back({x, y});
      m_terrain[idx] = static_cast<uint8_t>(type);
      assigned[idx] = true;

      // Grow cluster
      size_t clusterIdx = 0;
      while (clusterIdx < cluster.size() &&
             static_cast<int>(cluster.size()) < dist.maxClusterSize) {
        Position pos = cluster[clusterIdx++];

        // Try to extend to neighbors
        for (int d = 0; d < 4; ++d) {
          Position neighbor = getNeighbor(pos, static_cast<Direction>(d));
          if (!isValid(neighbor))
            continue;

          size_t nIdx = index(neighbor.x, neighbor.y);
          if (assigned[nIdx])
            continue;

          // Random chance to extend cluster
          if (uniform(rng) < dist.clusterChance) {
            m_terrain[nIdx] = static_cast<uint8_t>(type);
            assigned[nIdx] = true;
            cluster.push_back(neighbor);
          }
        }
      }
    }
  }

  // Ensure start and end are always normal terrain
  setTerrain(m_start, TerrainType::Normal);
  setTerrain(m_end, TerrainType::Normal);
}

void Grid::clearTerrain() {
  std::fill(m_terrain.begin(), m_terrain.end(),
            static_cast<uint8_t>(TerrainType::Normal));
}

bool Grid::saveToFile(const std::string &filename) const {
  std::ofstream file(filename);
  if (!file.is_open())
    return false;

  // Header: Width Height StartX StartY EndX EndY
  file << m_width << " " << m_height << "\n";
  file << m_start.x << " " << m_start.y << "\n";
  file << m_end.x << " " << m_end.y << "\n";

  // Data blocks
  for (int y = 0; y < m_height; ++y) {
    for (int x = 0; x < m_width; ++x) {
      uint8_t cell = getCell(x, y);
      uint8_t terrain = m_terrain[index(x, y)];

      // Format: Hex bitflag (2 chars) + Terrain ID (1 char) + Space
      file << std::hex << static_cast<int>(cell) << " " << std::dec
           << static_cast<int>(terrain) << " ";
    }
    file << "\n";
  }

  file.close();
  return true;
}

bool Grid::loadFromFile(const std::string &filename) {
  std::ifstream file(filename);
  if (!file.is_open())
    return false;

  int w, h;
  if (!(file >> w >> h))
    return false;

  // Create a temporary vector to avoid corrupting current state on failure
  std::vector<uint8_t> newCells(w * h);
  std::vector<uint8_t> newTerrain(w * h);

  Position start, end;
  if (!(file >> start.x >> start.y))
    return false;
  if (!(file >> end.x >> end.y))
    return false;

  for (int y = 0; y < h; ++y) {
    for (int x = 0; x < w; ++x) {
      int cellVal, terrainVal;
      if (!(file >> std::hex >> cellVal >> std::dec >> terrainVal)) {
        return false;
      }
      newCells[y * w + x] = static_cast<uint8_t>(cellVal);
      newTerrain[y * w + x] = static_cast<uint8_t>(terrainVal);
    }
  }

  // Assign on success
  m_width = w;
  m_height = h;
  m_start = start;
  m_end = end;
  m_cells = std::move(newCells);
  m_terrain = std::move(newTerrain);

  file.close();
  return true;
}

} // namespace maze
