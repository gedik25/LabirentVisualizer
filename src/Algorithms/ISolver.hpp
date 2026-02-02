#pragma once

#include "../Core/Grid.hpp"
#include <vector>
#include <string>
#include <memory>

namespace maze {

/**
 * Abstract interface for pathfinding algorithms.
 * All solvers must implement this interface for consistent behavior.
 * 
 * Usage:
 *   1. Create solver with grid reference
 *   2. Call init() to set start and end positions
 *   3. Call step() repeatedly for animated solving, or solve() for instant result
 *   4. Check isFinished() and foundPath() for status
 *   5. Use getPath() to get the solution path
 */
class ISolver {
public:
    virtual ~ISolver() = default;
    
    /**
     * Initialize the solver with start and end positions.
     * This resets any previous solving state.
     * 
     * @param start Starting position
     * @param end Target position
     */
    virtual void init(const Position& start, const Position& end) = 0;
    
    /**
     * Perform one step of the algorithm.
     * Call this in a loop for animated visualization.
     * 
     * @return true if algorithm is still running, false if finished
     */
    virtual bool step() = 0;
    
    /**
     * Run the complete algorithm without animation.
     * 
     * @return true if a path was found, false otherwise
     */
    virtual bool solve() = 0;
    
    /**
     * Reset the solver state.
     * Also clears solver-related flags from the grid.
     */
    virtual void reset() = 0;
    
    /**
     * Check if the algorithm has finished (found path or exhausted search).
     */
    virtual bool isFinished() const = 0;
    
    /**
     * Check if a valid path was found.
     * Only meaningful after isFinished() returns true.
     */
    virtual bool foundPath() const = 0;
    
    /**
     * Get the solution path from start to end.
     * Empty if no path found or algorithm not finished.
     */
    virtual const std::vector<Position>& getPath() const = 0;
    
    /**
     * Get all visited cells during the search.
     * Useful for visualization.
     */
    virtual const std::vector<Position>& getVisited() const = 0;
    
    /**
     * Get the current position being processed.
     * Useful for visualization.
     */
    virtual Position getCurrentPosition() const = 0;
    
    /**
     * Get the algorithm name for display.
     */
    virtual std::string getName() const = 0;
    
    /**
     * Get statistics about the search.
     */
    struct Stats {
        size_t nodesVisited = 0;
        size_t nodesInQueue = 0;
        size_t pathLength = 0;
        float elapsedTime = 0.0f; // in milliseconds
    };
    virtual Stats getStats() const = 0;
};

/**
 * Base class with common functionality for solvers.
 * Derived classes should override the pure virtual methods.
 */
class SolverBase : public ISolver {
public:
    explicit SolverBase(std::shared_ptr<Grid> grid)
        : m_grid(std::move(grid))
        , m_finished(false)
        , m_pathFound(false)
        , m_current{0, 0}
    {}
    
    void reset() override {
        m_visited.clear();
        m_path.clear();
        m_finished = false;
        m_pathFound = false;
        m_stats = Stats{};
        
        if (m_grid) {
            m_grid->clearSolverState();
        }
    }
    
    bool isFinished() const override { return m_finished; }
    bool foundPath() const override { return m_pathFound; }
    const std::vector<Position>& getPath() const override { return m_path; }
    const std::vector<Position>& getVisited() const override { return m_visited; }
    Position getCurrentPosition() const override { return m_current; }
    Stats getStats() const override { return m_stats; }

protected:
    std::shared_ptr<Grid> m_grid;
    std::vector<Position> m_visited;
    std::vector<Position> m_path;
    Position m_start;
    Position m_end;
    Position m_current;
    bool m_finished;
    bool m_pathFound;
    Stats m_stats;
    
    /**
     * Reconstruct path from parent map.
     * Called when end is reached.
     */
    void reconstructPath(const std::vector<std::vector<Position>>& parent) {
        m_path.clear();
        Position pos = m_end;
        
        while (pos != m_start) {
            m_path.push_back(pos);
            m_grid->setFlag(pos, CellFlags::InPath);
            pos = parent[pos.y][pos.x];
        }
        m_path.push_back(m_start);
        m_grid->setFlag(m_start, CellFlags::InPath);
        
        // Reverse to get start-to-end order
        std::reverse(m_path.begin(), m_path.end());
        m_stats.pathLength = m_path.size();
    }
};

} // namespace maze
