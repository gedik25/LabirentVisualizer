#include "EllersGenerator.hpp"
#include <algorithm>

namespace maze {

void EllersGenerator::init(std::shared_ptr<Grid> grid, unsigned int seed) {
    GeneratorBase::init(grid, seed);
    
    int width = m_grid->getWidth();
    
    // Initialize first row - each cell in its own set
    m_rowSets.resize(width);
    m_nextSetId = 0;
    for (int x = 0; x < width; ++x) {
        m_rowSets[x] = m_nextSetId++;
    }
    
    m_currentRow = 0;
    m_currentCol = 0;
    m_horizontalPhase = true;
    m_generating = true;
    m_current = {0, 0};
}

void EllersGenerator::initRow() {
    int width = m_grid->getWidth();
    
    // Assign new set IDs to cells that don't have one
    // (cells without downward passage from previous row)
    for (int x = 0; x < width; ++x) {
        if (m_rowSets[x] == -1) {
            m_rowSets[x] = m_nextSetId++;
        }
    }
}

bool EllersGenerator::sameSet(int col1, int col2) const {
    if (col1 < 0 || col1 >= static_cast<int>(m_rowSets.size())) return false;
    if (col2 < 0 || col2 >= static_cast<int>(m_rowSets.size())) return false;
    return m_rowSets[col1] == m_rowSets[col2];
}

void EllersGenerator::mergeSets(int fromSet, int toSet) {
    for (int& setId : m_rowSets) {
        if (setId == fromSet) {
            setId = toSet;
        }
    }
}

bool EllersGenerator::stepHorizontal() {
    int width = m_grid->getWidth();
    int height = m_grid->getHeight();
    bool isLastRow = (m_currentRow == height - 1);
    
    // Clear current marker
    m_grid->clearFlag(m_current, CellFlags::Current);
    
    // Process current cell
    if (m_currentCol < width - 1) {
        Position pos{m_currentCol, m_currentRow};
        Position rightPos{m_currentCol + 1, m_currentRow};
        
        m_current = pos;
        m_grid->setFlag(pos, CellFlags::Current);
        markVisited(pos);
        
        // If last row, must merge all cells into same set
        // Otherwise, randomly decide to merge
        if (!sameSet(m_currentCol, m_currentCol + 1)) {
            bool shouldMerge = isLastRow;
            
            if (!isLastRow) {
                std::uniform_int_distribution<int> dist(0, 1);
                shouldMerge = (dist(m_rng) == 0);
            }
            
            if (shouldMerge) {
                // Remove wall between cells
                m_grid->removeWallBetween(pos, rightPos);
                // Merge sets
                mergeSets(m_rowSets[m_currentCol + 1], m_rowSets[m_currentCol]);
            }
        }
        
        m_currentCol++;
        ++m_cellsProcessed;
        return true;
    }
    
    // Mark last cell in row
    Position lastPos{width - 1, m_currentRow};
    markVisited(lastPos);
    
    // Horizontal phase complete for this row
    if (isLastRow) {
        // Last row - we're done
        m_complete = true;
        m_generating = false;
        clearGenerationMarkers();
        return false;
    }
    
    // Switch to vertical phase
    m_horizontalPhase = false;
    m_currentCol = 0;
    return true;
}

bool EllersGenerator::stepVertical() {
    int width = m_grid->getWidth();
    
    // Clear current marker
    m_grid->clearFlag(m_current, CellFlags::Current);
    
    // Group cells by set
    std::map<int, std::vector<int>> setToCols;
    for (int x = 0; x < width; ++x) {
        setToCols[m_rowSets[x]].push_back(x);
    }
    
    // For each set, ensure at least one downward passage
    std::vector<int> nextRowSets(width, -1);
    
    for (auto& [setId, cols] : setToCols) {
        // Randomly decide which cells get downward passages
        // At least one must
        std::vector<bool> carveDown(cols.size(), false);
        
        // Randomly select cells to carve down
        int numCarved = 0;
        for (size_t i = 0; i < cols.size(); ++i) {
            std::uniform_int_distribution<int> dist(0, 1);
            if (dist(m_rng) == 0) {
                carveDown[i] = true;
                numCarved++;
            }
        }
        
        // Ensure at least one
        if (numCarved == 0) {
            std::uniform_int_distribution<size_t> dist(0, cols.size() - 1);
            carveDown[dist(m_rng)] = true;
        }
        
        // Apply downward passages
        for (size_t i = 0; i < cols.size(); ++i) {
            int col = cols[i];
            if (carveDown[i]) {
                Position pos{col, m_currentRow};
                Position downPos{col, m_currentRow + 1};
                m_grid->removeWallBetween(pos, downPos);
                nextRowSets[col] = setId;  // Carry set to next row
            }
        }
    }
    
    // Move to next row
    m_rowSets = nextRowSets;
    m_currentRow++;
    m_currentCol = 0;
    
    // Initialize new row (assign new set IDs to unconnected cells)
    initRow();
    
    // Switch back to horizontal phase
    m_horizontalPhase = true;
    
    m_current = {0, m_currentRow};
    ++m_cellsProcessed;
    
    return true;
}

bool EllersGenerator::step() {
    if (m_complete || !m_generating) {
        return false;
    }
    
    if (m_horizontalPhase) {
        return stepHorizontal();
    } else {
        return stepVertical();
    }
}

void EllersGenerator::generateFull() {
    while (step()) {
        // Continue until complete
    }
}

void EllersGenerator::reset() {
    GeneratorBase::reset();
    m_rowSets.clear();
    m_currentRow = 0;
    m_currentCol = 0;
    m_nextSetId = 0;
    m_horizontalPhase = true;
}

} // namespace maze
