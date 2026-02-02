#include "GreedySolver.hpp"
#include <chrono>
#include <cmath>

namespace maze {

GreedySolver::GreedySolver(std::shared_ptr<Grid> grid)
    : SolverBase(std::move(grid))
{
}

void GreedySolver::init(const Position& start, const Position& end) {
    reset();
    
    m_start = start;
    m_end = end;
    m_current = start;
    
    initDataStructures();
    
    // Initialize start node
    m_openSet.push({start, heuristic(start, end)});
    m_inOpenSet[start.y][start.x] = true;
    m_grid->setFlag(start, CellFlags::Queued);
    m_stats.nodesInQueue = 1;
}

void GreedySolver::initDataStructures() {
    int width = m_grid->getWidth();
    int height = m_grid->getHeight();
    
    // Initialize parent tracking
    m_parent.assign(height, std::vector<Position>(width, {-1, -1}));
    
    // Initialize sets
    m_inOpenSet.assign(height, std::vector<bool>(width, false));
    m_visited_map.assign(height, std::vector<bool>(width, false));
    
    // Clear priority queue
    while (!m_openSet.empty()) {
        m_openSet.pop();
    }
}

float GreedySolver::heuristic(const Position& a, const Position& b) const {
    // Manhattan distance
    return static_cast<float>(std::abs(a.x - b.x) + std::abs(a.y - b.y));
}

bool GreedySolver::step() {
    if (m_finished) {
        return false;
    }
    
    auto startTime = std::chrono::high_resolution_clock::now();
    
    if (m_openSet.empty()) {
        // No path exists
        m_finished = true;
        m_pathFound = false;
        return false;
    }
    
    // Clear current marker from previous position
    m_grid->clearFlag(m_current, CellFlags::Current);
    
    // Get node with smallest heuristic
    Node currentNode = m_openSet.top();
    m_openSet.pop();
    m_current = currentNode.pos;
    
    // Skip if already visited (duplicate in queue)
    if (m_visited_map[m_current.y][m_current.x]) {
        auto endTime = std::chrono::high_resolution_clock::now();
        m_stats.elapsedTime += std::chrono::duration<float, std::milli>(endTime - startTime).count();
        return true;
    }
    
    // Mark as visited
    m_visited_map[m_current.y][m_current.x] = true;
    m_inOpenSet[m_current.y][m_current.x] = false;
    m_grid->clearFlag(m_current, CellFlags::Queued);
    m_grid->setFlag(m_current, CellFlags::Visited);
    m_grid->setFlag(m_current, CellFlags::Current);
    m_visited.push_back(m_current);
    ++m_stats.nodesVisited;
    
    // Update queue size
    m_stats.nodesInQueue = m_openSet.size();
    
    // Check if we reached the end
    if (m_current == m_end) {
        m_finished = true;
        m_pathFound = true;
        reconstructPath(m_parent);
        m_grid->clearFlag(m_current, CellFlags::Current);
        
        auto endTime = std::chrono::high_resolution_clock::now();
        m_stats.elapsedTime += std::chrono::duration<float, std::milli>(endTime - startTime).count();
        return false;
    }
    
    // Explore neighbors
    std::vector<Position> neighbors = m_grid->getAccessibleNeighbors(m_current);
    
    for (const Position& neighbor : neighbors) {
        // Skip if already visited
        if (m_visited_map[neighbor.y][neighbor.x]) {
            continue;
        }
        
        // Add to open set if not already there
        if (!m_inOpenSet[neighbor.y][neighbor.x]) {
            m_parent[neighbor.y][neighbor.x] = m_current;
            m_openSet.push({neighbor, heuristic(neighbor, m_end)});
            m_inOpenSet[neighbor.y][neighbor.x] = true;
            m_grid->setFlag(neighbor, CellFlags::Queued);
        }
    }
    
    auto endTime = std::chrono::high_resolution_clock::now();
    m_stats.elapsedTime += std::chrono::duration<float, std::milli>(endTime - startTime).count();
    
    return true; // Still searching
}

bool GreedySolver::solve() {
    while (step()) {
        // Continue until finished
    }
    return m_pathFound;
}

void GreedySolver::reset() {
    SolverBase::reset();
    
    while (!m_openSet.empty()) {
        m_openSet.pop();
    }
    m_parent.clear();
    m_inOpenSet.clear();
    m_visited_map.clear();
}

} // namespace maze
