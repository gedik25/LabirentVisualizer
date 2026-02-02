#pragma once

#include "ISolver.hpp"
#include <queue>

namespace maze {

/**
 * Bidirectional BFS pathfinding algorithm.
 * Searches from both start and end simultaneously until they meet.
 * Can be up to 2x faster than regular BFS.
 * 
 * Characteristics:
 * - Uses two queues (forward and backward)
 * - Alternates between forward and backward search
 * - Meets in the middle, reducing search space
 * - Optimal for unweighted graphs
 * - Memory: O(V) where V is number of vertices
 * - Time: O(b^(d/2)) vs O(b^d) for regular BFS (where b=branching, d=depth)
 */
class BidirectionalBFS : public SolverBase {
public:
    explicit BidirectionalBFS(std::shared_ptr<Grid> grid);
    
    void init(const Position& start, const Position& end) override;
    bool step() override;
    bool solve() override;
    void reset() override;
    std::string getName() const override { return "Bidirectional BFS"; }

private:
    // Forward search (from start)
    std::queue<Position> m_forwardQueue;
    std::vector<std::vector<bool>> m_forwardVisited;
    std::vector<std::vector<Position>> m_forwardParent;
    
    // Backward search (from end)
    std::queue<Position> m_backwardQueue;
    std::vector<std::vector<bool>> m_backwardVisited;
    std::vector<std::vector<Position>> m_backwardParent;
    
    // Meeting point
    Position m_meetingPoint;
    bool m_foundMeeting;
    
    // Alternating search direction
    bool m_forwardTurn;
    
    void initDataStructures();
    
    // Perform one step of forward search
    bool stepForward();
    
    // Perform one step of backward search
    bool stepBackward();
    
    // Check if position was visited by other search
    bool checkMeeting(const Position& pos, bool isForward);
    
    // Reconstruct path when meeting point is found
    void reconstructBidirectionalPath();
};

} // namespace maze
