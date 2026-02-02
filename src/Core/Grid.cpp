#include "Grid.hpp"

namespace maze {

Grid::Grid(int width, int height)
    : m_width(width)
    , m_height(height)
    , m_cells(static_cast<size_t>(width * height), CellFlags::AllWalls)
    , m_start{0, 0}
    , m_end{width - 1, height - 1}
{
    if (width <= 0 || height <= 0) {
        throw std::invalid_argument("Grid dimensions must be positive");
    }
}

bool Grid::isValid(int x, int y) const {
    return x >= 0 && x < m_width && y >= 0 && y < m_height;
}

bool Grid::isValid(const Position& pos) const {
    return isValid(pos.x, pos.y);
}

uint8_t Grid::getCell(int x, int y) const {
    if (!isValid(x, y)) {
        throw std::out_of_range("Cell position out of bounds");
    }
    return m_cells[index(x, y)];
}

uint8_t Grid::getCell(const Position& pos) const {
    return getCell(pos.x, pos.y);
}

void Grid::setCell(int x, int y, uint8_t value) {
    if (!isValid(x, y)) {
        throw std::out_of_range("Cell position out of bounds");
    }
    m_cells[index(x, y)] = value;
}

void Grid::setCell(const Position& pos, uint8_t value) {
    setCell(pos.x, pos.y, value);
}

bool Grid::hasFlag(int x, int y, uint8_t flag) const {
    return maze::hasFlag(getCell(x, y), flag);
}

bool Grid::hasFlag(const Position& pos, uint8_t flag) const {
    return hasFlag(pos.x, pos.y, flag);
}

void Grid::setFlag(int x, int y, uint8_t flag) {
    if (!isValid(x, y)) return;
    maze::setFlag(m_cells[index(x, y)], flag);
}

void Grid::setFlag(const Position& pos, uint8_t flag) {
    setFlag(pos.x, pos.y, flag);
}

void Grid::clearFlag(int x, int y, uint8_t flag) {
    if (!isValid(x, y)) return;
    maze::clearFlag(m_cells[index(x, y)], flag);
}

void Grid::clearFlag(const Position& pos, uint8_t flag) {
    clearFlag(pos.x, pos.y, flag);
}

bool Grid::hasWall(int x, int y, Direction dir) const {
    return hasFlag(x, y, wallFlagForDirection(dir));
}

bool Grid::hasWall(const Position& pos, Direction dir) const {
    return hasWall(pos.x, pos.y, dir);
}

void Grid::setWall(int x, int y, Direction dir) {
    setFlag(x, y, wallFlagForDirection(dir));
}

void Grid::removeWall(int x, int y, Direction dir) {
    clearFlag(x, y, wallFlagForDirection(dir));
}

void Grid::removeWallBetween(const Position& from, const Position& to) {
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

bool Grid::canMove(const Position& from, Direction dir) const {
    if (!isValid(from)) return false;
    if (hasWall(from, dir)) return false;
    
    Position neighbor = getNeighbor(from, dir);
    return isValid(neighbor);
}

bool Grid::canMove(const Position& from, const Position& to) const {
    int dx = to.x - from.x;
    int dy = to.y - from.y;
    
    // Must be adjacent
    if (std::abs(dx) + std::abs(dy) != 1) return false;
    
    Direction dir;
    if (dx == 1) dir = Direction::East;
    else if (dx == -1) dir = Direction::West;
    else if (dy == 1) dir = Direction::South;
    else dir = Direction::North;
    
    return canMove(from, dir);
}

Position Grid::getNeighbor(const Position& pos, Direction dir) const {
    Position offset = directionOffset(dir);
    return {pos.x + offset.x, pos.y + offset.y};
}

std::vector<Position> Grid::getNeighbors(const Position& pos) const {
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

std::vector<Position> Grid::getAccessibleNeighbors(const Position& pos) const {
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
}

void Grid::clearSolverState() {
    constexpr uint8_t solverFlags = CellFlags::Visited | CellFlags::InPath | 
                                     CellFlags::Current | CellFlags::Queued;
    for (auto& cell : m_cells) {
        cell &= ~solverFlags;
    }
}

} // namespace maze
