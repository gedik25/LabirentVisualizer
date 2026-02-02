#pragma once

#include <SFML/Graphics/View.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <algorithm>

namespace maze {

/**
 * Camera system for maze visualization.
 * Supports panning, zooming, and viewport bounds.
 */
class Camera {
public:
    Camera(float windowWidth, float windowHeight);
    
    // Update camera view based on current state
    void update();
    
    // Apply camera view to render window
    void apply(sf::RenderWindow& window) const;
    
    // Pan camera by delta
    void pan(float dx, float dy);
    
    // Zoom controls
    void zoomIn();
    void zoomOut();
    void setZoom(float zoom);
    float getZoom() const { return m_zoom; }
    
    // Set/get position (center of view)
    void setPosition(float x, float y);
    sf::Vector2f getPosition() const { return m_position; }
    
    // Set window size (call on window resize)
    void setWindowSize(float width, float height);
    sf::Vector2f getWindowSize() const { return m_windowSize; }
    
    // Fit the entire maze in view
    void fitToMaze(int mazeWidth, int mazeHeight, float cellSize);
    
    // Get view bounds in world coordinates
    struct Bounds {
        float left, top, right, bottom;
    };
    Bounds getViewBounds() const;
    
    // Get visible cell range for culling
    struct CellRange {
        int minX, minY, maxX, maxY;
    };
    CellRange getVisibleCells(float cellSize, int gridWidth, int gridHeight) const;
    
    // Mouse position to world position
    sf::Vector2f screenToWorld(const sf::Vector2i& screenPos) const;
    
    // Get the SFML view
    const sf::View& getView() const { return m_view; }
    
    // Pan speed (pixels per frame at zoom 1.0)
    float panSpeed = 10.0f;
    
    // Zoom constraints
    static constexpr float MinZoom = 0.1f;
    static constexpr float MaxZoom = 10.0f;
    static constexpr float ZoomStep = 1.1f;

private:
    sf::View m_view;
    sf::Vector2f m_position;
    sf::Vector2f m_windowSize;
    float m_zoom;
};

} // namespace maze
