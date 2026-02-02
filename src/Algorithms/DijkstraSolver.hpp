#pragma once

#include "ISolver.hpp"
#include <queue>
#include <functional>

namespace maze {

/**
 * Dijkstra's pathfinding algorithm.
 * Finds shortest path in weighted graphs without heuristic.
 * Essentially A* with h(n) = 0.
 * 
 * Characteristics:
 * - Uses priority queue ordered by distance from start
 * - Explores all directions uniformly
 * - Optimal for weighted graphs
 * - More thorough than A* but slower
 * - Memory: O(V) where V is number of vertices
 * - Time: O(E log V) where E is number of edges
 */
class DijkstraSolver : public SolverBase {
public:
    explicit DijkstraSolver(std::shared_ptr<Grid> grid);
    
    void init(const Position& start, const Position& end) override;
    bool step() override;
    bool solve() override;
    void reset() override;
    std::string getName() const override { return "Dijkstra"; }

private:
    struct Node {
        Position pos;
        float distance;
        
        bool operator>(const Node& other) const {
            return distance > other.distance;
        }
    };
    
    std::priority_queue<Node, std::vector<Node>, std::greater<Node>> m_openSet;
    std::vector<std::vector<float>> m_distance;
    std::vector<std::vector<Position>> m_parent;
    std::vector<std::vector<bool>> m_inOpenSet;
    std::vector<std::vector<bool>> m_processed;
    
    void initDataStructures();
    
    // Get movement cost (for future terrain support)
    float getMovementCost(const Position& from, const Position& to) const;
};

} // namespace maze
