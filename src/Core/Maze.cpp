#include "Maze.hpp"
#include <algorithm>
#include <chrono>

namespace maze {

Maze::Maze(std::shared_ptr<Grid> grid)
    : m_grid(std::move(grid))
    , m_current{0, 0}
    , m_generating(false)
    , m_complete(false)
{
    // Seed with current time by default
    auto seed = static_cast<unsigned int>(
        std::chrono::steady_clock::now().time_since_epoch().count()
    );
    m_rng.seed(seed);
}

void Maze::startGeneration(const Position& startPos) {
    // Reset the grid to all walls
    m_grid->reset();
    
    // Clear the stack
    while (!m_stack.empty()) {
        m_stack.pop();
    }
    
    // Initialize starting position
    m_current = startPos;
    m_stack.push(m_current);
    markVisited(m_current);
    
    m_generating = true;
    m_complete = false;
}

bool Maze::step() {
    if (!m_generating || m_complete) {
        return false;
    }
    
    // Clear current marker from previous position
    m_grid->clearFlag(m_current, CellFlags::Current);
    
    // Get unvisited neighbors of current cell
    std::vector<Position> unvisited = getUnvisitedNeighbors(m_current);
    
    if (!unvisited.empty()) {
        // Choose random unvisited neighbor
        Position next = chooseRandomNeighbor(unvisited);
        
        // Push current cell to stack
        m_stack.push(m_current);
        
        // Remove wall between current and chosen cell
        m_grid->removeWallBetween(m_current, next);
        
        // Move to chosen cell
        m_current = next;
        markVisited(m_current);
        
    } else if (!m_stack.empty()) {
        // Backtrack: pop from stack
        m_current = m_stack.top();
        m_stack.pop();
        
    } else {
        // Stack is empty and no unvisited neighbors - generation complete
        m_complete = true;
        m_generating = false;
        
        // Clear all generation markers
        for (int y = 0; y < m_grid->getHeight(); ++y) {
            for (int x = 0; x < m_grid->getWidth(); ++x) {
                m_grid->clearFlag(x, y, CellFlags::Visited | CellFlags::Current);
            }
        }
        
        return false;
    }
    
    // Mark current position
    m_grid->setFlag(m_current, CellFlags::Current);
    
    return true;
}

void Maze::generateFull() {
    startGeneration({0, 0});
    while (step()) {
        // Continue until complete
    }
}

void Maze::reset() {
    m_grid->reset();
    while (!m_stack.empty()) {
        m_stack.pop();
    }
    m_current = {0, 0};
    m_generating = false;
    m_complete = false;
}

void Maze::setSeed(unsigned int seed) {
    m_rng.seed(seed);
}

void Maze::markVisited(const Position& pos) {
    m_grid->setFlag(pos, CellFlags::Visited);
}

bool Maze::isVisited(const Position& pos) const {
    return m_grid->hasFlag(pos, CellFlags::Visited);
}

std::vector<Position> Maze::getUnvisitedNeighbors(const Position& pos) const {
    std::vector<Position> unvisited;
    unvisited.reserve(4);
    
    // Check all four directions
    for (int d = 0; d < 4; ++d) {
        Direction dir = static_cast<Direction>(d);
        Position neighbor = m_grid->getNeighbor(pos, dir);
        
        if (m_grid->isValid(neighbor) && !isVisited(neighbor)) {
            unvisited.push_back(neighbor);
        }
    }
    
    return unvisited;
}

Position Maze::chooseRandomNeighbor(const std::vector<Position>& neighbors) {
    if (neighbors.empty()) {
        return {-1, -1}; // Invalid position
    }
    
    std::uniform_int_distribution<size_t> dist(0, neighbors.size() - 1);
    return neighbors[dist(m_rng)];
}

} // namespace maze
