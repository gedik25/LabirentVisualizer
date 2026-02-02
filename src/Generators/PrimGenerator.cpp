#include "PrimGenerator.hpp"
#include <algorithm>

namespace maze {

void PrimGenerator::init(std::shared_ptr<Grid> grid, unsigned int seed) {
    GeneratorBase::init(grid, seed);
    
    m_frontier.clear();
    
    // Start from a random cell
    std::uniform_int_distribution<int> distX(0, m_grid->getWidth() - 1);
    std::uniform_int_distribution<int> distY(0, m_grid->getHeight() - 1);
    
    m_current = {distX(m_rng), distY(m_rng)};
    
    // Mark starting cell as visited
    markVisited(m_current);
    m_grid->setFlag(m_current, CellFlags::Current);
    ++m_cellsProcessed;
    
    // Add walls of starting cell to frontier
    addWallsToFrontier(m_current);
    
    m_generating = true;
}

void PrimGenerator::addWallsToFrontier(const Position& pos) {
    // Check all four directions
    for (int d = 0; d < 4; ++d) {
        Direction dir = static_cast<Direction>(d);
        Position neighbor = m_grid->getNeighbor(pos, dir);
        
        // Add wall if neighbor is valid and not visited
        if (m_grid->isValid(neighbor) && !isVisited(neighbor)) {
            m_frontier.push_back({pos, neighbor});
        }
    }
}

size_t PrimGenerator::pickRandomWall() {
    if (m_frontier.empty()) return 0;
    
    std::uniform_int_distribution<size_t> dist(0, m_frontier.size() - 1);
    return dist(m_rng);
}

bool PrimGenerator::step() {
    if (m_complete || !m_generating) {
        return false;
    }
    
    // Clear current marker
    m_grid->clearFlag(m_current, CellFlags::Current);
    
    // Remove invalid walls (both cells visited)
    m_frontier.erase(
        std::remove_if(m_frontier.begin(), m_frontier.end(),
            [this](const Wall& w) {
                return isVisited(w.cell1) && isVisited(w.cell2);
            }),
        m_frontier.end()
    );
    
    if (m_frontier.empty()) {
        // Generation complete
        m_complete = true;
        m_generating = false;
        clearGenerationMarkers();
        return false;
    }
    
    // Pick a random wall
    size_t idx = pickRandomWall();
    Wall wall = m_frontier[idx];
    
    // Remove this wall from frontier
    m_frontier.erase(m_frontier.begin() + idx);
    
    // Determine which cell is new (unvisited)
    Position newCell;
    Position existingCell;
    
    if (!isVisited(wall.cell2)) {
        newCell = wall.cell2;
        existingCell = wall.cell1;
    } else if (!isVisited(wall.cell1)) {
        newCell = wall.cell1;
        existingCell = wall.cell2;
    } else {
        // Both visited, skip
        return true;
    }
    
    // Remove wall between cells
    m_grid->removeWallBetween(existingCell, newCell);
    
    // Mark new cell as visited
    markVisited(newCell);
    m_current = newCell;
    m_grid->setFlag(m_current, CellFlags::Current);
    ++m_cellsProcessed;
    
    // Add new cell's walls to frontier
    addWallsToFrontier(newCell);
    
    return true;
}

void PrimGenerator::generateFull() {
    while (step()) {
        // Continue until complete
    }
}

void PrimGenerator::reset() {
    GeneratorBase::reset();
    m_frontier.clear();
}

} // namespace maze
