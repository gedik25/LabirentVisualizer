#include "BFSSolver.hpp"
#include <chrono>

namespace maze {

BFSSolver::BFSSolver(std::shared_ptr<Grid> grid)
    : SolverBase(std::move(grid))
{
}

void BFSSolver::init(const Position& start, const Position& end) {
    reset();
    
    m_start = start;
    m_end = end;
    m_current = start;
    
    initDataStructures();
    
    // Add start to queue
    m_queue.push(m_start);
    m_inQueue[m_start.y][m_start.x] = true;
    m_grid->setFlag(m_start, CellFlags::Queued);
    m_stats.nodesInQueue = 1;
}

void BFSSolver::initDataStructures() {
    int width = m_grid->getWidth();
    int height = m_grid->getHeight();
    
    // Initialize parent tracking
    m_parent.assign(height, std::vector<Position>(width, {-1, -1}));
    
    // Initialize queue tracking
    m_inQueue.assign(height, std::vector<bool>(width, false));
    
    // Clear queue
    while (!m_queue.empty()) {
        m_queue.pop();
    }
}

bool BFSSolver::step() {
    if (m_finished) {
        return false;
    }
    
    auto startTime = std::chrono::high_resolution_clock::now();
    
    if (m_queue.empty()) {
        // No path exists
        m_finished = true;
        m_pathFound = false;
        return false;
    }
    
    // Clear current marker from previous position
    m_grid->clearFlag(m_current, CellFlags::Current);
    
    // Dequeue front cell
    m_current = m_queue.front();
    m_queue.pop();
    m_stats.nodesInQueue = m_queue.size();
    
    // Mark as visited
    m_grid->clearFlag(m_current, CellFlags::Queued);
    m_grid->setFlag(m_current, CellFlags::Visited);
    m_grid->setFlag(m_current, CellFlags::Current);
    m_visited.push_back(m_current);
    ++m_stats.nodesVisited;
    
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
        if (!m_inQueue[neighbor.y][neighbor.x] && 
            !m_grid->hasFlag(neighbor, CellFlags::Visited)) {
            
            m_queue.push(neighbor);
            m_inQueue[neighbor.y][neighbor.x] = true;
            m_parent[neighbor.y][neighbor.x] = m_current;
            m_grid->setFlag(neighbor, CellFlags::Queued);
            ++m_stats.nodesInQueue;
        }
    }
    
    auto endTime = std::chrono::high_resolution_clock::now();
    m_stats.elapsedTime += std::chrono::duration<float, std::milli>(endTime - startTime).count();
    
    return true; // Still searching
}

bool BFSSolver::solve() {
    while (step()) {
        // Continue until finished
    }
    return m_pathFound;
}

void BFSSolver::reset() {
    SolverBase::reset();
    
    while (!m_queue.empty()) {
        m_queue.pop();
    }
    m_parent.clear();
    m_inQueue.clear();
}

} // namespace maze
