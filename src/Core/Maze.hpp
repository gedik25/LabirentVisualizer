#pragma once

#include "Grid.hpp"
#include <stack>
#include <random>
#include <memory>

namespace maze {

/**
 * Maze generator using Recursive Backtracking algorithm.
 * Supports step-by-step generation for animation.
 */
class Maze {
public:
    explicit Maze(std::shared_ptr<Grid> grid);
    
    // Start maze generation from a specific position (default: 0,0)
    void startGeneration(const Position& startPos = {0, 0});
    
    // Perform one step of maze generation
    // Returns true if generation is still in progress
    bool step();
    
    // Generate the entire maze at once (no animation)
    void generateFull();
    
    // Check if generation is complete
    bool isComplete() const { return m_complete; }
    
    // Check if generation has started
    bool isGenerating() const { return m_generating; }
    
    // Get current position being processed (for visualization)
    Position getCurrentPosition() const { return m_current; }
    
    // Get the grid
    std::shared_ptr<Grid> getGrid() const { return m_grid; }
    
    // Reset for new generation
    void reset();
    
    // Set random seed for reproducible mazes
    void setSeed(unsigned int seed);

private:
    std::shared_ptr<Grid> m_grid;
    std::stack<Position> m_stack;
    Position m_current;
    bool m_generating;
    bool m_complete;
    std::mt19937 m_rng;
    
    // Mark cell as visited during generation
    void markVisited(const Position& pos);
    bool isVisited(const Position& pos) const;
    
    // Get unvisited neighbors
    std::vector<Position> getUnvisitedNeighbors(const Position& pos) const;
    
    // Choose random neighbor
    Position chooseRandomNeighbor(const std::vector<Position>& neighbors);
};

} // namespace maze
