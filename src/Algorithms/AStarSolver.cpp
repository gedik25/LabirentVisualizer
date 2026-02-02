#include "AStarSolver.hpp"
#include <chrono>
#include <cmath>
#include <limits>

namespace maze {

AStarSolver::AStarSolver(std::shared_ptr<Grid> grid)
    : SolverBase(std::move(grid))
{
}

void AStarSolver::init(const Position& start, const Position& end) {
    reset();
    
    m_start = start;
    m_end = end;
    m_current = start;
    
    initDataStructures();
    
    // Initialize start node
    m_gScore[start.y][start.x] = 0;
    m_fScore[start.y][start.x] = heuristic(start, end);
    
    m_openSet.push({start, m_fScore[start.y][start.x]});
    m_inOpenSet[start.y][start.x] = true;
    m_grid->setFlag(start, CellFlags::Queued);
    m_stats.nodesInQueue = 1;
}

void AStarSolver::initDataStructures() {
    int width = m_grid->getWidth();
    int height = m_grid->getHeight();
    
    constexpr float INF = std::numeric_limits<float>::infinity();
    
    // Initialize scores with infinity
    m_gScore.assign(height, std::vector<float>(width, INF));
    m_fScore.assign(height, std::vector<float>(width, INF));
    
    // Initialize parent tracking
    m_parent.assign(height, std::vector<Position>(width, {-1, -1}));
    
    // Initialize sets
    m_inOpenSet.assign(height, std::vector<bool>(width, false));
    m_closedSet.assign(height, std::vector<bool>(width, false));
    
    // Clear priority queue
    while (!m_openSet.empty()) {
        m_openSet.pop();
    }
}

float AStarSolver::heuristic(const Position& a, const Position& b) const {
    // Manhattan distance - admissible heuristic for grid movement
    return static_cast<float>(std::abs(a.x - b.x) + std::abs(a.y - b.y));
}

float AStarSolver::getMovementCost(const Position& from, const Position& to) const {
    // Use terrain cost from grid
    return m_grid->getMovementCost(from, to);
}

bool AStarSolver::step() {
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
    
    // Get node with lowest f-score
    Node currentNode = m_openSet.top();
    m_openSet.pop();
    m_current = currentNode.pos;
    
    // Skip if already processed (duplicate in queue)
    if (m_closedSet[m_current.y][m_current.x]) {
        auto endTime = std::chrono::high_resolution_clock::now();
        m_stats.elapsedTime += std::chrono::duration<float, std::milli>(endTime - startTime).count();
        return true;
    }
    
    // Mark as visited (closed set)
    m_closedSet[m_current.y][m_current.x] = true;
    m_inOpenSet[m_current.y][m_current.x] = false;
    m_grid->clearFlag(m_current, CellFlags::Queued);
    m_grid->setFlag(m_current, CellFlags::Visited);
    m_grid->setFlag(m_current, CellFlags::Current);
    m_visited.push_back(m_current);
    ++m_stats.nodesVisited;
    
    // Update queue size (approximate)
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
        // Skip if already in closed set
        if (m_closedSet[neighbor.y][neighbor.x]) {
            continue;
        }
        
        // Calculate tentative g-score
        float tentativeG = m_gScore[m_current.y][m_current.x] + 
                          getMovementCost(m_current, neighbor);
        
        // Check if this path is better
        if (tentativeG < m_gScore[neighbor.y][neighbor.x]) {
            // This is a better path
            m_parent[neighbor.y][neighbor.x] = m_current;
            m_gScore[neighbor.y][neighbor.x] = tentativeG;
            m_fScore[neighbor.y][neighbor.x] = tentativeG + heuristic(neighbor, m_end);
            
            // Add to open set if not already there
            if (!m_inOpenSet[neighbor.y][neighbor.x]) {
                m_openSet.push({neighbor, m_fScore[neighbor.y][neighbor.x]});
                m_inOpenSet[neighbor.y][neighbor.x] = true;
                m_grid->setFlag(neighbor, CellFlags::Queued);
            }
        }
    }
    
    auto endTime = std::chrono::high_resolution_clock::now();
    m_stats.elapsedTime += std::chrono::duration<float, std::milli>(endTime - startTime).count();
    
    return true; // Still searching
}

bool AStarSolver::solve() {
    while (step()) {
        // Continue until finished
    }
    return m_pathFound;
}

void AStarSolver::reset() {
    SolverBase::reset();
    
    while (!m_openSet.empty()) {
        m_openSet.pop();
    }
    m_gScore.clear();
    m_fScore.clear();
    m_parent.clear();
    m_inOpenSet.clear();
    m_closedSet.clear();
}

} // namespace maze
