#pragma once

#include "../Core/Grid.hpp"
#include <memory>
#include <string>
#include <random>

namespace maze {

/**
 * Abstract interface for maze generation algorithms.
 * All generators must implement this interface for consistent behavior.
 * 
 * Usage:
 *   1. Create generator
 *   2. Call init() with grid and seed
 *   3. Call step() repeatedly for animated generation, or generateFull() for instant
 *   4. Check isComplete() for status
 */
class IMazeGenerator {
public:
    virtual ~IMazeGenerator() = default;
    
    /**
     * Initialize the generator with a grid and random seed.
     * 
     * @param grid Shared pointer to the grid
     * @param seed Random seed for reproducible mazes
     */
    virtual void init(std::shared_ptr<Grid> grid, unsigned int seed) = 0;
    
    /**
     * Perform one step of maze generation.
     * Call this in a loop for animated visualization.
     * 
     * @return true if generation is still in progress, false if complete
     */
    virtual bool step() = 0;
    
    /**
     * Generate the entire maze at once (no animation).
     */
    virtual void generateFull() = 0;
    
    /**
     * Reset the generator for new generation.
     */
    virtual void reset() = 0;
    
    /**
     * Check if generation is complete.
     */
    virtual bool isComplete() const = 0;
    
    /**
     * Check if generation has started.
     */
    virtual bool isGenerating() const = 0;
    
    /**
     * Get current position being processed (for visualization).
     */
    virtual Position getCurrentPosition() const = 0;
    
    /**
     * Get the algorithm name for display.
     */
    virtual std::string getName() const = 0;
    
    /**
     * Get generation statistics.
     */
    struct Stats {
        size_t cellsProcessed = 0;
        size_t totalCells = 0;
        float progress = 0.0f;  // 0.0 to 1.0
    };
    virtual Stats getStats() const = 0;
};

/**
 * Base class with common functionality for maze generators.
 */
class GeneratorBase : public IMazeGenerator {
public:
    GeneratorBase() = default;
    
    void init(std::shared_ptr<Grid> grid, unsigned int seed) override {
        m_grid = grid;
        m_rng.seed(seed);
        m_seed = seed;
        m_generating = false;
        m_complete = false;
        m_current = {0, 0};
        m_cellsProcessed = 0;
        
        if (m_grid) {
            m_grid->reset();  // Start with all walls
            m_totalCells = static_cast<size_t>(m_grid->getWidth() * m_grid->getHeight());
        }
    }
    
    void reset() override {
        m_generating = false;
        m_complete = false;
        m_current = {0, 0};
        m_cellsProcessed = 0;
        
        if (m_grid) {
            m_grid->reset();
        }
    }
    
    bool isComplete() const override { return m_complete; }
    bool isGenerating() const override { return m_generating; }
    Position getCurrentPosition() const override { return m_current; }
    
    Stats getStats() const override {
        Stats stats;
        stats.cellsProcessed = m_cellsProcessed;
        stats.totalCells = m_totalCells;
        stats.progress = m_totalCells > 0 ? 
            static_cast<float>(m_cellsProcessed) / m_totalCells : 0.0f;
        return stats;
    }

protected:
    std::shared_ptr<Grid> m_grid;
    std::mt19937 m_rng;
    unsigned int m_seed = 0;
    Position m_current{0, 0};
    bool m_generating = false;
    bool m_complete = false;
    size_t m_cellsProcessed = 0;
    size_t m_totalCells = 0;
    
    // Mark cell as visited during generation
    void markVisited(const Position& pos) {
        if (m_grid && m_grid->isValid(pos)) {
            m_grid->setFlag(pos, CellFlags::Visited);
        }
    }
    
    bool isVisited(const Position& pos) const {
        return m_grid && m_grid->isValid(pos) && 
               m_grid->hasFlag(pos, CellFlags::Visited);
    }
    
    // Clear generation markers when complete
    void clearGenerationMarkers() {
        if (!m_grid) return;
        
        for (int y = 0; y < m_grid->getHeight(); ++y) {
            for (int x = 0; x < m_grid->getWidth(); ++x) {
                m_grid->clearFlag(x, y, CellFlags::Visited | CellFlags::Current);
            }
        }
    }
};

} // namespace maze
