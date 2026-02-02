#pragma once

#include "IMazeGenerator.hpp"
#include <vector>

namespace maze {

/**
 * Prim's Algorithm maze generator.
 * Creates mazes using a randomized version of Prim's minimum spanning tree algorithm.
 * 
 * Characteristics:
 * - Starts from a random cell and grows outward
 * - Produces mazes with many short dead ends
 * - More "blobby" appearance compared to Recursive Backtracking
 * - Uses a frontier (wall list) data structure
 */
class PrimGenerator : public GeneratorBase {
public:
    PrimGenerator() = default;
    
    void init(std::shared_ptr<Grid> grid, unsigned int seed) override;
    bool step() override;
    void generateFull() override;
    void reset() override;
    std::string getName() const override { return "Prim's Algorithm"; }

private:
    // Wall structure: connects two cells
    struct Wall {
        Position cell1;
        Position cell2;
    };
    
    std::vector<Wall> m_frontier;  // List of walls to consider
    
    // Add walls of a cell to the frontier
    void addWallsToFrontier(const Position& pos);
    
    // Pick a random wall from frontier
    size_t pickRandomWall();
};

} // namespace maze
