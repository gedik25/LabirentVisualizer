#include "BidirectionalBFS.hpp"
#include <chrono>
#include <algorithm>

namespace maze {

BidirectionalBFS::BidirectionalBFS(std::shared_ptr<Grid> grid)
    : SolverBase(std::move(grid))
    , m_meetingPoint{-1, -1}
    , m_foundMeeting(false)
    , m_forwardTurn(true)
{
}

void BidirectionalBFS::init(const Position& start, const Position& end) {
    reset();
    
    m_start = start;
    m_end = end;
    m_current = start;
    
    initDataStructures();
    
    // Initialize forward search from start
    m_forwardQueue.push(start);
    m_forwardVisited[start.y][start.x] = true;
    m_grid->setFlag(start, CellFlags::Queued);
    
    // Initialize backward search from end
    m_backwardQueue.push(end);
    m_backwardVisited[end.y][end.x] = true;
    m_grid->setFlag(end, CellFlags::Queued);
    
    m_stats.nodesInQueue = 2;
    m_forwardTurn = true;
}

void BidirectionalBFS::initDataStructures() {
    int width = m_grid->getWidth();
    int height = m_grid->getHeight();
    
    // Initialize forward search structures
    m_forwardVisited.assign(height, std::vector<bool>(width, false));
    m_forwardParent.assign(height, std::vector<Position>(width, {-1, -1}));
    
    // Initialize backward search structures
    m_backwardVisited.assign(height, std::vector<bool>(width, false));
    m_backwardParent.assign(height, std::vector<Position>(width, {-1, -1}));
    
    // Clear queues
    while (!m_forwardQueue.empty()) m_forwardQueue.pop();
    while (!m_backwardQueue.empty()) m_backwardQueue.pop();
    
    m_meetingPoint = {-1, -1};
    m_foundMeeting = false;
}

bool BidirectionalBFS::step() {
    if (m_finished) {
        return false;
    }
    
    auto startTime = std::chrono::high_resolution_clock::now();
    
    // Check if both queues are empty
    if (m_forwardQueue.empty() && m_backwardQueue.empty()) {
        m_finished = true;
        m_pathFound = false;
        return false;
    }
    
    // Clear current marker
    m_grid->clearFlag(m_current, CellFlags::Current);
    
    bool result;
    
    // Alternate between forward and backward search
    if (m_forwardTurn && !m_forwardQueue.empty()) {
        result = stepForward();
    } else if (!m_forwardTurn && !m_backwardQueue.empty()) {
        result = stepBackward();
    } else if (!m_forwardQueue.empty()) {
        result = stepForward();
    } else if (!m_backwardQueue.empty()) {
        result = stepBackward();
    } else {
        m_finished = true;
        m_pathFound = false;
        return false;
    }
    
    m_forwardTurn = !m_forwardTurn;
    
    // Update queue size
    m_stats.nodesInQueue = m_forwardQueue.size() + m_backwardQueue.size();
    
    auto endTime = std::chrono::high_resolution_clock::now();
    m_stats.elapsedTime += std::chrono::duration<float, std::milli>(endTime - startTime).count();
    
    return result;
}

bool BidirectionalBFS::stepForward() {
    if (m_forwardQueue.empty()) return true;
    
    Position current = m_forwardQueue.front();
    m_forwardQueue.pop();
    m_current = current;
    
    // Mark as visited
    m_grid->setFlag(current, CellFlags::Visited);
    m_grid->setFlag(current, CellFlags::Current);
    m_visited.push_back(current);
    ++m_stats.nodesVisited;
    
    // Check if we met the backward search
    if (checkMeeting(current, true)) {
        return false;
    }
    
    // Explore neighbors
    std::vector<Position> neighbors = m_grid->getAccessibleNeighbors(current);
    
    for (const Position& neighbor : neighbors) {
        if (!m_forwardVisited[neighbor.y][neighbor.x]) {
            m_forwardVisited[neighbor.y][neighbor.x] = true;
            m_forwardParent[neighbor.y][neighbor.x] = current;
            m_forwardQueue.push(neighbor);
            m_grid->setFlag(neighbor, CellFlags::Queued);
            
            // Check meeting immediately
            if (checkMeeting(neighbor, true)) {
                return false;
            }
        }
    }
    
    return true;
}

bool BidirectionalBFS::stepBackward() {
    if (m_backwardQueue.empty()) return true;
    
    Position current = m_backwardQueue.front();
    m_backwardQueue.pop();
    m_current = current;
    
    // Mark as visited
    m_grid->setFlag(current, CellFlags::Visited);
    m_grid->setFlag(current, CellFlags::Current);
    m_visited.push_back(current);
    ++m_stats.nodesVisited;
    
    // Check if we met the forward search
    if (checkMeeting(current, false)) {
        return false;
    }
    
    // Explore neighbors
    std::vector<Position> neighbors = m_grid->getAccessibleNeighbors(current);
    
    for (const Position& neighbor : neighbors) {
        if (!m_backwardVisited[neighbor.y][neighbor.x]) {
            m_backwardVisited[neighbor.y][neighbor.x] = true;
            m_backwardParent[neighbor.y][neighbor.x] = current;
            m_backwardQueue.push(neighbor);
            m_grid->setFlag(neighbor, CellFlags::Queued);
            
            // Check meeting immediately
            if (checkMeeting(neighbor, false)) {
                return false;
            }
        }
    }
    
    return true;
}

bool BidirectionalBFS::checkMeeting(const Position& pos, bool isForward) {
    if (isForward) {
        // Check if backward search visited this position
        if (m_backwardVisited[pos.y][pos.x]) {
            m_meetingPoint = pos;
            m_foundMeeting = true;
            m_finished = true;
            m_pathFound = true;
            reconstructBidirectionalPath();
            m_grid->clearFlag(m_current, CellFlags::Current);
            return true;
        }
    } else {
        // Check if forward search visited this position
        if (m_forwardVisited[pos.y][pos.x]) {
            m_meetingPoint = pos;
            m_foundMeeting = true;
            m_finished = true;
            m_pathFound = true;
            reconstructBidirectionalPath();
            m_grid->clearFlag(m_current, CellFlags::Current);
            return true;
        }
    }
    return false;
}

void BidirectionalBFS::reconstructBidirectionalPath() {
    m_path.clear();
    
    // Build forward path (start to meeting point)
    std::vector<Position> forwardPath;
    Position pos = m_meetingPoint;
    while (pos.x != -1 && pos.y != -1) {
        forwardPath.push_back(pos);
        m_grid->setFlag(pos, CellFlags::InPath);
        if (pos == m_start) break;
        pos = m_forwardParent[pos.y][pos.x];
    }
    std::reverse(forwardPath.begin(), forwardPath.end());
    
    // Build backward path (meeting point to end)
    std::vector<Position> backwardPath;
    pos = m_backwardParent[m_meetingPoint.y][m_meetingPoint.x];
    while (pos.x != -1 && pos.y != -1) {
        backwardPath.push_back(pos);
        m_grid->setFlag(pos, CellFlags::InPath);
        if (pos == m_end) break;
        pos = m_backwardParent[pos.y][pos.x];
    }
    
    // Combine paths
    m_path = forwardPath;
    for (const auto& p : backwardPath) {
        m_path.push_back(p);
    }
    
    m_stats.pathLength = m_path.size();
}

bool BidirectionalBFS::solve() {
    while (step()) {
        // Continue until finished
    }
    return m_pathFound;
}

void BidirectionalBFS::reset() {
    SolverBase::reset();
    
    while (!m_forwardQueue.empty()) m_forwardQueue.pop();
    while (!m_backwardQueue.empty()) m_backwardQueue.pop();
    
    m_forwardVisited.clear();
    m_forwardParent.clear();
    m_backwardVisited.clear();
    m_backwardParent.clear();
    
    m_meetingPoint = {-1, -1};
    m_foundMeeting = false;
    m_forwardTurn = true;
}

} // namespace maze
