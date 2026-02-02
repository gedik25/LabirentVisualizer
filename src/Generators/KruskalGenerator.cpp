#include "KruskalGenerator.hpp"
#include <algorithm>

namespace maze {

void KruskalGenerator::init(std::shared_ptr<Grid> grid, unsigned int seed) {
    GeneratorBase::init(grid, seed);
    
    int width = m_grid->getWidth();
    int height = m_grid->getHeight();
    int totalCells = width * height;
    
    // Initialize Union-Find
    m_parent.resize(totalCells);
    m_rank.resize(totalCells, 0);
    for (int i = 0; i < totalCells; ++i) {
        m_parent[i] = i;  // Each cell is its own set initially
    }
    
    // Create all edges (internal walls)
    m_edges.clear();
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            Position pos{x, y};
            
            // Add edge to the right (East)
            if (x < width - 1) {
                m_edges.push_back({pos, {x + 1, y}});
            }
            
            // Add edge to the bottom (South)
            if (y < height - 1) {
                m_edges.push_back({pos, {x, y + 1}});
            }
        }
    }
    
    // Shuffle edges randomly
    shuffleEdges();
    
    m_edgeIndex = 0;
    m_generating = true;
    m_current = {0, 0};
}

void KruskalGenerator::shuffleEdges() {
    // Fisher-Yates shuffle
    for (size_t i = m_edges.size() - 1; i > 0; --i) {
        std::uniform_int_distribution<size_t> dist(0, i);
        size_t j = dist(m_rng);
        std::swap(m_edges[i], m_edges[j]);
    }
}

int KruskalGenerator::posToIndex(const Position& pos) const {
    return pos.y * m_grid->getWidth() + pos.x;
}

int KruskalGenerator::find(int x) {
    // Path compression
    if (m_parent[x] != x) {
        m_parent[x] = find(m_parent[x]);
    }
    return m_parent[x];
}

void KruskalGenerator::unite(int x, int y) {
    int rootX = find(x);
    int rootY = find(y);
    
    if (rootX == rootY) return;
    
    // Union by rank
    if (m_rank[rootX] < m_rank[rootY]) {
        m_parent[rootX] = rootY;
    } else if (m_rank[rootX] > m_rank[rootY]) {
        m_parent[rootY] = rootX;
    } else {
        m_parent[rootY] = rootX;
        m_rank[rootX]++;
    }
}

bool KruskalGenerator::connected(int x, int y) {
    return find(x) == find(y);
}

bool KruskalGenerator::step() {
    if (m_complete || !m_generating) {
        return false;
    }
    
    // Clear current marker
    m_grid->clearFlag(m_current, CellFlags::Current);
    
    // Find next edge that connects two different sets
    while (m_edgeIndex < m_edges.size()) {
        const Edge& edge = m_edges[m_edgeIndex++];
        
        int idx1 = posToIndex(edge.cell1);
        int idx2 = posToIndex(edge.cell2);
        
        // If cells are in different sets, remove the wall
        if (!connected(idx1, idx2)) {
            // Unite the sets
            unite(idx1, idx2);
            
            // Remove wall between cells
            m_grid->removeWallBetween(edge.cell1, edge.cell2);
            
            // Mark cells
            markVisited(edge.cell1);
            markVisited(edge.cell2);
            
            m_current = edge.cell2;
            m_grid->setFlag(m_current, CellFlags::Current);
            ++m_cellsProcessed;
            
            return true;
        }
    }
    
    // All edges processed
    m_complete = true;
    m_generating = false;
    clearGenerationMarkers();
    return false;
}

void KruskalGenerator::generateFull() {
    while (step()) {
        // Continue until complete
    }
}

void KruskalGenerator::reset() {
    GeneratorBase::reset();
    m_edges.clear();
    m_parent.clear();
    m_rank.clear();
    m_edgeIndex = 0;
}

} // namespace maze
