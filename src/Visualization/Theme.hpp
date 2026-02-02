#pragma once

#include <SFML/Graphics/Color.hpp>

namespace maze {

/**
 * Color theme for the maze visualizer.
 * Dark mode theme with neon accent colors.
 */
struct Theme {
    // Background
    sf::Color background{0x12, 0x12, 0x12};      // #121212 - Dark background
    
    // Maze structure
    sf::Color wall{0x33, 0x33, 0x33};            // #333333 - Dark grey walls
    sf::Color cellEmpty{0x1E, 0x1E, 0x1E};       // #1E1E1E - Empty cell floor
    sf::Color cellBorder{0x2A, 0x2A, 0x2A};      // #2A2A2A - Cell border
    
    // Maze generation
    sf::Color generation{0x00, 0xFF, 0x00};      // #00FF00 - Bright green for current path
    sf::Color generationVisited{0x00, 0x64, 0x00}; // #006400 - Dark green for visited
    
    // BFS visualization
    sf::Color bfsVisited{0x00, 0xE5, 0xFF};      // #00E5FF - Bright cyan
    sf::Color bfsCurrent{0x00, 0xB8, 0xD4};      // #00B8D4 - Darker cyan for current
    sf::Color bfsQueued{0x00, 0x7A, 0x8C};       // #007A8C - Even darker for queued
    
    // DFS visualization
    sf::Color dfsVisited{0xFF, 0xD6, 0x00};      // #FFD600 - Bright yellow
    sf::Color dfsCurrent{0xFF, 0xAB, 0x00};      // #FFAB00 - Orange for current
    sf::Color dfsQueued{0xC4, 0x8A, 0x00};       // #C48A00 - Dark gold for queued
    
    // Solution path
    sf::Color path{0xFF, 0x00, 0xFF};            // #FF00FF - Magenta
    sf::Color pathGlow{0xFF, 0x80, 0xFF};        // #FF80FF - Light magenta glow
    
    // Start and end markers
    sf::Color start{0x00, 0xFF, 0x7F};           // #00FF7F - Spring green
    sf::Color end{0xFF, 0x45, 0x00};             // #FF4500 - Orange red
    
    // UI elements
    sf::Color textPrimary{0xFF, 0xFF, 0xFF};     // #FFFFFF - White text
    sf::Color textSecondary{0xAA, 0xAA, 0xAA};   // #AAAAAA - Grey text
    sf::Color textHighlight{0x00, 0xE5, 0xFF};   // #00E5FF - Cyan highlight
    
    // Get color for visited cells based on algorithm type
    enum class AlgorithmType {
        BFS,
        DFS
    };
    
    sf::Color getVisitedColor(AlgorithmType type) const {
        return (type == AlgorithmType::BFS) ? bfsVisited : dfsVisited;
    }
    
    sf::Color getCurrentColor(AlgorithmType type) const {
        return (type == AlgorithmType::BFS) ? bfsCurrent : dfsCurrent;
    }
    
    sf::Color getQueuedColor(AlgorithmType type) const {
        return (type == AlgorithmType::BFS) ? bfsQueued : dfsQueued;
    }
};

// Global theme instance
inline const Theme& getTheme() {
    static Theme theme;
    return theme;
}

} // namespace maze
