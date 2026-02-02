#include "BinaryTreeGenerator.hpp"

namespace maze {

void BinaryTreeGenerator::init(std::shared_ptr<Grid> grid, unsigned int seed) {
    GeneratorBase::init(grid, seed);
    
    m_currentX = 0;
    m_currentY = 0;
    m_current = {0, 0};
    
    m_generating = true;
}

bool BinaryTreeGenerator::step() {
    if (m_complete || !m_generating) {
        return false;
    }
    
    // Clear current marker
    m_grid->clearFlag(m_current, CellFlags::Current);
    
    int width = m_grid->getWidth();
    int height = m_grid->getHeight();
    
    // Check if we've processed all cells
    if (m_currentY >= height) {
        m_complete = true;
        m_generating = false;
        clearGenerationMarkers();
        return false;
    }
    
    Position pos{m_currentX, m_currentY};
    m_current = pos;
    
    // Determine available directions (North and East)
    bool canGoNorth = (m_currentY > 0);
    bool canGoEast = (m_currentX < width - 1);
    
    if (canGoNorth && canGoEast) {
        // Randomly choose north or east
        std::uniform_int_distribution<int> dist(0, 1);
        if (dist(m_rng) == 0) {
            // Go North
            m_grid->removeWallBetween(pos, {m_currentX, m_currentY - 1});
        } else {
            // Go East
            m_grid->removeWallBetween(pos, {m_currentX + 1, m_currentY});
        }
    } else if (canGoNorth) {
        // Can only go North
        m_grid->removeWallBetween(pos, {m_currentX, m_currentY - 1});
    } else if (canGoEast) {
        // Can only go East
        m_grid->removeWallBetween(pos, {m_currentX + 1, m_currentY});
    }
    // If neither (top-right corner), do nothing
    
    markVisited(pos);
    m_grid->setFlag(pos, CellFlags::Current);
    ++m_cellsProcessed;
    
    // Move to next cell (row by row, left to right)
    m_currentX++;
    if (m_currentX >= width) {
        m_currentX = 0;
        m_currentY++;
    }
    
    return true;
}

void BinaryTreeGenerator::generateFull() {
    while (step()) {
        // Continue until complete
    }
}

void BinaryTreeGenerator::reset() {
    GeneratorBase::reset();
    m_currentX = 0;
    m_currentY = 0;
}

} // namespace maze
