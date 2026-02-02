#pragma once

#include "ISolver.hpp"
#include <queue>
#include <functional>

namespace maze {

/**
 * Greedy Best-First Search pathfinding algorithm.
 * Uses only heuristic to guide search, ignoring actual path cost.
 * Fast but does NOT guarantee optimal path.
 * 
 * f(n) = h(n) only (no g-score)
 * 
 * Characteristics:
 * - Uses priority queue ordered by heuristic
 * - Very fast, explores towards goal directly
 * - NOT optimal - can find suboptimal paths
 * - Good for quick approximations
 * - Memory: O(V) where V is number of vertices
 * - Time: O(E log V) where E is number of edges
 */
class GreedySolver : public SolverBase {
public:
    explicit GreedySolver(std::shared_ptr<Grid> grid);
    
    void init(const Position& start, const Position& end) override;
    bool step() override;
    bool solve() override;
    void reset() override;
    std::string getName() const override { return "Greedy Best-First"; }

private:
    struct Node {
        Position pos;
        float hScore;  // Heuristic only
        
        bool operator>(const Node& other) const {
            return hScore > other.hScore;
        }
    };
    
    std::priority_queue<Node, std::vector<Node>, std::greater<Node>> m_openSet;
    std::vector<std::vector<Position>> m_parent;
    std::vector<std::vector<bool>> m_inOpenSet;
    std::vector<std::vector<bool>> m_visited_map;
    
    void initDataStructures();
    
    // Heuristic function (Manhattan distance)
    float heuristic(const Position& a, const Position& b) const;
};

} // namespace maze
