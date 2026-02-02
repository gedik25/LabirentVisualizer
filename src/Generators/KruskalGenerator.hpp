#pragma once

#include "IMazeGenerator.hpp"
#include <vector>

namespace maze {

/**
 * Kruskal's Algorithm maze generator.
 * Creates mazes using a randomized version of Kruskal's minimum spanning tree algorithm.
 * Uses Union-Find (Disjoint Set) data structure.
 * 
 * Characteristics:
 * - Randomly removes walls that connect different regions
 * - More uniform distribution of passages
 * - Produces mazes similar to Prim's but with different characteristics
 * - Uses Union-Find for efficient set operations
 */
class KruskalGenerator : public GeneratorBase {
public:
    KruskalGenerator() = default;
    
    void init(std::shared_ptr<Grid> grid, unsigned int seed) override;
    bool step() override;
    void generateFull() override;
    void reset() override;
    std::string getName() const override { return "Kruskal's Algorithm"; }

private:
    // Edge (wall) between two cells
    struct Edge {
        Position cell1;
        Position cell2;
    };
    
    std::vector<Edge> m_edges;      // All possible edges (walls)
    std::vector<int> m_parent;      // Union-Find parent array
    std::vector<int> m_rank;        // Union-Find rank array
    size_t m_edgeIndex;             // Current edge being processed
    
    // Union-Find operations
    int find(int x);
    void unite(int x, int y);
    bool connected(int x, int y);
    
    // Convert position to index
    int posToIndex(const Position& pos) const;
    
    // Shuffle edges randomly
    void shuffleEdges();
};

} // namespace maze
