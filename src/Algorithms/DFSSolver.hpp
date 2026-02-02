#pragma once

#include "ISolver.hpp"
#include <stack>

namespace maze {

/**
 * Depth-First Search pathfinding algorithm.
 * Explores as far as possible along each branch before backtracking.
 * Does NOT guarantee the shortest path.
 * 
 * Characteristics:
 * - Uses a stack (LIFO) data structure
 * - Explores deeply before broadly
 * - Memory efficient for sparse graphs
 * - Memory: O(V) where V is number of vertices
 * - Time: O(V + E) where E is number of edges
 */
class DFSSolver : public SolverBase {
public:
    explicit DFSSolver(std::shared_ptr<Grid> grid);
    
    void init(const Position& start, const Position& end) override;
    bool step() override;
    bool solve() override;
    void reset() override;
    std::string getName() const override { return "DFS (Depth-First Search)"; }

private:
    std::stack<Position> m_stack;
    std::vector<std::vector<Position>> m_parent;
    std::vector<std::vector<bool>> m_onStack;
    
    void initDataStructures();
};

} // namespace maze
