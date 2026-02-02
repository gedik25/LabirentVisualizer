#pragma once

#include "Cell.hpp"
#include <vector>
#include <stdexcept>

namespace maze {

/**
 * Memory-efficient grid storage for maze cells.
 * Uses a flat vector with 1 byte per cell (bit flags for walls and states).
 * Supports rectangular grids of any size.
 */
class Grid {
public:
    Grid(int width, int height);
    
    // Dimensions
    int getWidth() const { return m_width; }
    int getHeight() const { return m_height; }
    size_t getCellCount() const { return m_cells.size(); }
    
    // Bounds checking
    bool isValid(int x, int y) const;
    bool isValid(const Position& pos) const;
    
    // Cell access
    uint8_t getCell(int x, int y) const;
    uint8_t getCell(const Position& pos) const;
    void setCell(int x, int y, uint8_t value);
    void setCell(const Position& pos, uint8_t value);
    
    // Flag operations
    bool hasFlag(int x, int y, uint8_t flag) const;
    bool hasFlag(const Position& pos, uint8_t flag) const;
    void setFlag(int x, int y, uint8_t flag);
    void setFlag(const Position& pos, uint8_t flag);
    void clearFlag(int x, int y, uint8_t flag);
    void clearFlag(const Position& pos, uint8_t flag);
    
    // Wall operations
    bool hasWall(int x, int y, Direction dir) const;
    bool hasWall(const Position& pos, Direction dir) const;
    void setWall(int x, int y, Direction dir);
    void removeWall(int x, int y, Direction dir);
    void removeWallBetween(const Position& from, const Position& to);
    
    // Check if can move from one cell to another (no wall blocking)
    bool canMove(const Position& from, Direction dir) const;
    bool canMove(const Position& from, const Position& to) const;
    
    // Get neighbor position in a direction
    Position getNeighbor(const Position& pos, Direction dir) const;
    
    // Get all valid neighbors (within bounds)
    std::vector<Position> getNeighbors(const Position& pos) const;
    
    // Get all accessible neighbors (no wall blocking)
    std::vector<Position> getAccessibleNeighbors(const Position& pos) const;
    
    // Reset operations
    void reset();          // Reset to all walls, clear solver state
    void clearSolverState(); // Clear only solver flags (Visited, InPath, Current, Queued)
    
    // Start and end positions
    Position getStart() const { return m_start; }
    Position getEnd() const { return m_end; }
    void setStart(const Position& pos) { m_start = pos; }
    void setEnd(const Position& pos) { m_end = pos; }

private:
    int m_width;
    int m_height;
    std::vector<uint8_t> m_cells;
    Position m_start;
    Position m_end;
    
    // Convert 2D coordinates to 1D index
    size_t index(int x, int y) const { return static_cast<size_t>(y * m_width + x); }
};

} // namespace maze
