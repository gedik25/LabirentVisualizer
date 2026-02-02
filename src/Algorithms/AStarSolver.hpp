#pragma once

#include "ISolver.hpp"
#include <queue>
#include <functional>

namespace maze {

/**
 * A* (A-Star) pathfinding algorithm.
 * Uses heuristic to guide search towards the goal efficiently.
 * Guarantees optimal path when using admissible heuristic.
 * 
 * f(n) = g(n) + h(n)
 * - g(n): actual cost from start to n
 * - h(n): heuristic estimate from n to goal
 * 
 * Characteristics:
 * - Uses priority queue ordered by f-score
 * - Explores most promising nodes first
 * - Optimal with admissible heuristic (Manhattan distance)
 * - Memory: O(V) where V is number of vertices
 * - Time: O(E log V) where E is number of edges
 */
class AStarSolver : public SolverBase {
public:
    explicit AStarSolver(std::shared_ptr<Grid> grid);
    
    void init(const Position& start, const Position& end) override;
    bool step() override;
    bool solve() override;
    void reset() override;
    std::string getName() const override { return "A* (A-Star)"; }

private:
    // Node with f-score for priority queue
    struct Node {
        Position pos;
        float fScore;  // f = g + h
        
        bool operator>(const Node& other) const {
            return fScore > other.fScore;
        }
    };
    
    std::priority_queue<Node, std::vector<Node>, std::greater<Node>> m_openSet;
    std::vector<std::vector<float>> m_gScore;   // Cost from start to node
    std::vector<std::vector<float>> m_fScore;   // f = g + h
    std::vector<std::vector<Position>> m_parent;
    std::vector<std::vector<bool>> m_inOpenSet;
    std::vector<std::vector<bool>> m_closedSet;
    
    void initDataStructures();
    
    // Heuristic function (Manhattan distance)
    float heuristic(const Position& a, const Position& b) const;
    
    // Get movement cost (for future terrain support)
    float getMovementCost(const Position& from, const Position& to) const;
};

} // namespace maze
