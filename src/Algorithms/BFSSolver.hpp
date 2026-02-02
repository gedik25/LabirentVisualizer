#pragma once

#include "ISolver.hpp"
#include <queue>

namespace maze {

/**
 * Breadth-First Search pathfinding algorithm.
 * Explores all nodes at the current depth before moving to next depth level.
 * Guarantees the shortest path in an unweighted graph.
 * 
 * Characteristics:
 * - Uses a queue (FIFO) data structure
 * - Explores layer by layer (wavefront expansion)
 * - Optimal for unweighted graphs
 * - Memory: O(V) where V is number of vertices
 * - Time: O(V + E) where E is number of edges
 */
class BFSSolver : public SolverBase {
public:
    explicit BFSSolver(std::shared_ptr<Grid> grid);
    
    void init(const Position& start, const Position& end) override;
    bool step() override;
    bool solve() override;
    void reset() override;
    std::string getName() const override { return "BFS (Breadth-First Search)"; }

private:
    std::queue<Position> m_queue;
    std::vector<std::vector<Position>> m_parent;
    std::vector<std::vector<bool>> m_inQueue;
    
    void initDataStructures();
};

} // namespace maze
