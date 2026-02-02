#include "DijkstraSolver.hpp"
#include <chrono>
#include <limits>

namespace maze {

DijkstraSolver::DijkstraSolver(std::shared_ptr<Grid> grid)
    : SolverBase(std::move(grid))
{
}

void DijkstraSolver::init(const Position& start, const Position& end) {
    reset();
    
    m_start = start;
    m_end = end;
    m_current = start;
    
    initDataStructures();
    
    // Initialize start node
    m_distance[start.y][start.x] = 0;
    
    m_openSet.push({start, 0});
    m_inOpenSet[start.y][start.x] = true;
    m_grid->setFlag(start, CellFlags::Queued);
    m_stats.nodesInQueue = 1;
}

void DijkstraSolver::initDataStructures() {
    int width = m_grid->getWidth();
    int height = m_grid->getHeight();
    
    constexpr float INF = std::numeric_limits<float>::infinity();
    
    // Initialize distances with infinity
    m_distance.assign(height, std::vector<float>(width, INF));
    
    // Initialize parent tracking
    m_parent.assign(height, std::vector<Position>(width, {-1, -1}));
    
    // Initialize sets
    m_inOpenSet.assign(height, std::vector<bool>(width, false));
    m_processed.assign(height, std::vector<bool>(width, false));
    
    // Clear priority queue
    while (!m_openSet.empty()) {
        m_openSet.pop();
    }
}

float DijkstraSolver::getMovementCost(const Position& from, const Position& to) const {
    // Use terrain cost from grid
    return m_grid->getMovementCost(from, to);
}

bool DijkstraSolver::step() {
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
    
    // Get node with smallest distance
    Node currentNode = m_openSet.top();
    m_openSet.pop();
    m_current = currentNode.pos;
    
    // Skip if already processed (duplicate in queue)
    if (m_processed[m_current.y][m_current.x]) {
        auto endTime = std::chrono::high_resolution_clock::now();
        m_stats.elapsedTime += std::chrono::duration<float, std::milli>(endTime - startTime).count();
        return true;
    }
    
    // Mark as processed
    m_processed[m_current.y][m_current.x] = true;
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
        // Skip if already processed
        if (m_processed[neighbor.y][neighbor.x]) {
            continue;
        }
        
        // Calculate tentative distance
        float tentativeDist = m_distance[m_current.y][m_current.x] + 
                             getMovementCost(m_current, neighbor);
        
        // Check if this path is better
        if (tentativeDist < m_distance[neighbor.y][neighbor.x]) {
            // This is a better path
            m_parent[neighbor.y][neighbor.x] = m_current;
            m_distance[neighbor.y][neighbor.x] = tentativeDist;
            
            // Add to open set
            m_openSet.push({neighbor, tentativeDist});
            if (!m_inOpenSet[neighbor.y][neighbor.x]) {
                m_inOpenSet[neighbor.y][neighbor.x] = true;
                m_grid->setFlag(neighbor, CellFlags::Queued);
            }
        }
    }
    
    auto endTime = std::chrono::high_resolution_clock::now();
    m_stats.elapsedTime += std::chrono::duration<float, std::milli>(endTime - startTime).count();
    
    return true; // Still searching
}

bool DijkstraSolver::solve() {
    while (step()) {
        // Continue until finished
    }
    return m_pathFound;
}

void DijkstraSolver::reset() {
    SolverBase::reset();
    
    while (!m_openSet.empty()) {
        m_openSet.pop();
    }
    m_distance.clear();
    m_parent.clear();
    m_inOpenSet.clear();
    m_processed.clear();
}

} // namespace maze
