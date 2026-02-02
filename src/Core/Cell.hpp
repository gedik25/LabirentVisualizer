#pragma once

#include <cstdint>

namespace maze {

// Position struct for grid coordinates
struct Position {
    int x;
    int y;
    
    bool operator==(const Position& other) const {
        return x == other.x && y == other.y;
    }
    
    bool operator!=(const Position& other) const {
        return !(*this == other);
    }
};

// Direction enum for maze navigation
enum class Direction : uint8_t {
    North = 0,
    East  = 1,
    South = 2,
    West  = 3
};

// Cell wall flags - using bit flags for memory efficiency
// Each cell stores which walls are present
namespace CellFlags {
    constexpr uint8_t None       = 0;
    constexpr uint8_t WallNorth  = 1 << 0;  // 1  - Top wall
    constexpr uint8_t WallEast   = 1 << 1;  // 2  - Right wall
    constexpr uint8_t WallSouth  = 1 << 2;  // 4  - Bottom wall
    constexpr uint8_t WallWest   = 1 << 3;  // 8  - Left wall
    constexpr uint8_t AllWalls   = WallNorth | WallEast | WallSouth | WallWest; // 15
    
    // Solver state flags (used during pathfinding)
    constexpr uint8_t Visited    = 1 << 4;  // 16 - Cell has been visited by solver
    constexpr uint8_t InPath     = 1 << 5;  // 32 - Cell is part of solution path
    constexpr uint8_t Current    = 1 << 6;  // 64 - Currently being processed
    constexpr uint8_t Queued     = 1 << 7;  // 128 - In the queue/stack
}

// Helper functions for cell flag manipulation
inline bool hasFlag(uint8_t cell, uint8_t flag) {
    return (cell & flag) != 0;
}

inline void setFlag(uint8_t& cell, uint8_t flag) {
    cell |= flag;
}

inline void clearFlag(uint8_t& cell, uint8_t flag) {
    cell &= ~flag;
}

inline void toggleFlag(uint8_t& cell, uint8_t flag) {
    cell ^= flag;
}

// Get the opposite direction
inline Direction oppositeDirection(Direction dir) {
    switch (dir) {
        case Direction::North: return Direction::South;
        case Direction::East:  return Direction::West;
        case Direction::South: return Direction::North;
        case Direction::West:  return Direction::East;
    }
    return Direction::North; // Should never reach here
}

// Get wall flag for a direction
inline uint8_t wallFlagForDirection(Direction dir) {
    switch (dir) {
        case Direction::North: return CellFlags::WallNorth;
        case Direction::East:  return CellFlags::WallEast;
        case Direction::South: return CellFlags::WallSouth;
        case Direction::West:  return CellFlags::WallWest;
    }
    return CellFlags::None;
}

// Get direction offset (dx, dy)
inline Position directionOffset(Direction dir) {
    switch (dir) {
        case Direction::North: return {0, -1};
        case Direction::East:  return {1, 0};
        case Direction::South: return {0, 1};
        case Direction::West:  return {-1, 0};
    }
    return {0, 0};
}

} // namespace maze
