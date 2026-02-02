#include "DFSSolver.hpp"
#include <chrono>

namespace maze {

DFSSolver::DFSSolver(std::shared_ptr<Grid> grid)
    : SolverBase(std::move(grid))
{
}

void DFSSolver::init(const Position& start, const Position& end) {
    reset();
    
    m_start = start;
    m_end = end;
    m_current = start;
    
    initDataStructures();
    
    // Push start to stack
    m_stack.push(m_start);
    m_onStack[m_start.y][m_start.x] = true;
    m_grid->setFlag(m_start, CellFlags::Queued);
    m_stats.nodesInQueue = 1;
}

void DFSSolver::initDataStructures() {
    int width = m_grid->getWidth();
    int height = m_grid->getHeight();
    
    // Initialize parent tracking
    m_parent.assign(height, std::vector<Position>(width, {-1, -1}));
    
    // Initialize stack tracking
    m_onStack.assign(height, std::vector<bool>(width, false));
    
    // Clear stack
    while (!m_stack.empty()) {
        m_stack.pop();
    }
}

bool DFSSolver::step() {
    if (m_finished) {
        return false;
    }
    
    auto startTime = std::chrono::high_resolution_clock::now();
    
    if (m_stack.empty()) {
        // No path exists
        m_finished = true;
        m_pathFound = false;
        return false;
    }
    
    // Clear current marker from previous position
    m_grid->clearFlag(m_current, CellFlags::Current);
    
    // Pop top cell from stack
    m_current = m_stack.top();
    m_stack.pop();
    m_stats.nodesInQueue = m_stack.size();
    
    // Skip if already visited
    if (m_grid->hasFlag(m_current, CellFlags::Visited)) {
        auto endTime = std::chrono::high_resolution_clock::now();
        m_stats.elapsedTime += std::chrono::duration<float, std::milli>(endTime - startTime).count();
        return true;
    }
    
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
    
    // Explore neighbors (push in reverse order for consistent direction preference)
    std::vector<Position> neighbors = m_grid->getAccessibleNeighbors(m_current);
    
    // Push neighbors in reverse to maintain left-to-right, top-to-bottom preference
    for (auto it = neighbors.rbegin(); it != neighbors.rend(); ++it) {
        const Position& neighbor = *it;
        if (!m_grid->hasFlag(neighbor, CellFlags::Visited) && 
            !m_onStack[neighbor.y][neighbor.x]) {
            
            m_stack.push(neighbor);
            m_onStack[neighbor.y][neighbor.x] = true;
            m_parent[neighbor.y][neighbor.x] = m_current;
            m_grid->setFlag(neighbor, CellFlags::Queued);
            ++m_stats.nodesInQueue;
        }
    }
    
    auto endTime = std::chrono::high_resolution_clock::now();
    m_stats.elapsedTime += std::chrono::duration<float, std::milli>(endTime - startTime).count();
    
    return true; // Still searching
}

bool DFSSolver::solve() {
    while (step()) {
        // Continue until finished
    }
    return m_pathFound;
}

void DFSSolver::reset() {
    SolverBase::reset();
    
    while (!m_stack.empty()) {
        m_stack.pop();
    }
    m_parent.clear();
    m_onStack.clear();
}

} // namespace maze
