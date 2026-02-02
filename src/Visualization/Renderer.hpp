#pragma once

#include "../Core/Grid.hpp"
#include "../Core/Terrain.hpp"
#include "Camera.hpp"
#include "Theme.hpp"
#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Text.hpp>
#include <memory>
#include <optional>

namespace maze {

/**
 * Renderer for maze visualization.
 * Supports viewport culling for large mazes.
 */
class Renderer {
public:
  Renderer(sf::RenderWindow &window, Camera &camera);

  // Set the grid to render
  void setGrid(std::shared_ptr<Grid> grid);

  // Main render function
  void render();

  // Clear the window with background color
  void clear();

  // Display the window
  void display();

  // Cell size in pixels (base size before zoom)
  void setCellSize(float size) { m_cellSize = size; }
  float getCellSize() const { return m_cellSize; }

  // Auto-calculate cell size based on window and grid
  void autoCalculateCellSize();

  // Wall thickness
  void setWallThickness(float thickness) { m_wallThickness = thickness; }
  float getWallThickness() const { return m_wallThickness; }

  // Set algorithm type for coloring
  void setAlgorithmType(Theme::AlgorithmType type) { m_algorithmType = type; }

  // Render terrain colors
  void renderTerrain();

  // Render path overlay
  void renderPath();

  // Render statistics
  struct RenderStats {
    int cellsRendered;
    int cellsTotal;
    float fps;
  };
  RenderStats getStats() const { return m_stats; }

  // UI overlay
  void renderUI(const std::string &status, int animationDelay);

private:
  sf::RenderWindow &m_window;
  Camera &m_camera;
  std::shared_ptr<Grid> m_grid;

  float m_cellSize = 20.0f;
  float m_wallThickness = 2.0f;
  Theme::AlgorithmType m_algorithmType = Theme::AlgorithmType::BFS;

  RenderStats m_stats{0, 0, 0.0f};
  sf::Clock m_fpsClock;
  int m_frameCount = 0;

  // Reusable shapes for performance
  sf::RectangleShape m_cellShape;
  sf::RectangleShape m_wallShape;
  sf::CircleShape m_markerShape;

  // Font for UI (optional)
  std::optional<sf::Font> m_font;

  // Render individual cell
  void renderCell(int x, int y);

  // Render cell floor (background)
  void renderCellFloor(int x, int y, const sf::Color &color);

  // Render cell walls
  void renderCellWalls(int x, int y);

  // Render start/end markers
  void renderMarkers();

  // Get color for cell based on its state
  sf::Color getCellColor(int x, int y) const;

  // Update FPS counter
  void updateFPS();
};

} // namespace maze
