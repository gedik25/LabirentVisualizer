#pragma once

#include "IMazeGenerator.hpp"

namespace maze {

/**
 * Binary Tree maze generator.
 * One of the simplest maze generation algorithms.
 * For each cell, randomly carves north or east passage.
 * 
 * Characteristics:
 * - Extremely simple and fast
 * - Has a diagonal bias (NE corner is always open corridor)
 * - Every cell has at most 2 exits (north and east)
 * - Not a "true" random maze due to bias
 * - Good for learning, less interesting visually
 */
class BinaryTreeGenerator : public GeneratorBase {
public:
    BinaryTreeGenerator() = default;
    
    void init(std::shared_ptr<Grid> grid, unsigned int seed) override;
    bool step() override;
    void generateFull() override;
    void reset() override;
    std::string getName() const override { return "Binary Tree"; }

private:
    int m_currentX;
    int m_currentY;
};

} // namespace maze
