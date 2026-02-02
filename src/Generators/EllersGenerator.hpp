#pragma once

#include "IMazeGenerator.hpp"
#include <vector>
#include <map>

namespace maze {

/**
 * Eller's Algorithm maze generator.
 * Generates mazes row by row with minimal memory usage O(width).
 * 
 * Characteristics:
 * - Memory efficient - only needs to track current row
 * - Can generate infinite height mazes
 * - Good balance between complexity and randomness
 * - Each row is processed in two phases: horizontal merge, then vertical carve
 */
class EllersGenerator : public GeneratorBase {
public:
    EllersGenerator() = default;
    
    void init(std::shared_ptr<Grid> grid, unsigned int seed) override;
    bool step() override;
    void generateFull() override;
    void reset() override;
    std::string getName() const override { return "Eller's Algorithm"; }

private:
    std::vector<int> m_rowSets;     // Set ID for each cell in current row
    int m_currentRow;
    int m_currentCol;
    int m_nextSetId;
    bool m_horizontalPhase;         // true = merging horizontally, false = carving down
    
    // Initialize a new row
    void initRow();
    
    // Horizontal phase: randomly merge adjacent cells in different sets
    bool stepHorizontal();
    
    // Vertical phase: ensure each set has at least one downward passage
    bool stepVertical();
    
    // Check if two cells are in the same set
    bool sameSet(int col1, int col2) const;
    
    // Merge two sets
    void mergeSets(int fromSet, int toSet);
};

} // namespace maze
