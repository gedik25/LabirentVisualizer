#include "Renderer.hpp"
#include <iomanip>
#include <sstream>

namespace maze {

Renderer::Renderer(sf::RenderWindow &window, Camera &camera)
    : m_window(window), m_camera(camera) {
  // Initialize reusable shapes
  m_cellShape.setSize({m_cellSize, m_cellSize});
  m_wallShape.setFillColor(getTheme().wall);
  m_markerShape.setRadius(m_cellSize / 4.0f);
}

void Renderer::setGrid(std::shared_ptr<Grid> grid) {
  m_grid = std::move(grid);
  if (m_grid) {
    m_stats.cellsTotal = m_grid->getWidth() * m_grid->getHeight();
  }
}

void Renderer::render() {
  if (!m_grid)
    return;

  // Apply camera view
  m_camera.apply(m_window);

  // Get visible cell range for culling
  Camera::CellRange range = m_camera.getVisibleCells(
      m_cellSize, m_grid->getWidth(), m_grid->getHeight());

  m_stats.cellsRendered = 0;

  // Render only visible cells
  for (int y = range.minY; y <= range.maxY; ++y) {
    for (int x = range.minX; x <= range.maxX; ++x) {
      renderCell(x, y);
      ++m_stats.cellsRendered;
    }
  }

  // Render terrain
  renderTerrain();

  // Render path overlay
  renderPath();

  // Render start/end markers
  renderMarkers();

  // Update FPS
  updateFPS();
}

void Renderer::clear() { m_window.clear(getTheme().background); }

void Renderer::display() { m_window.display(); }

void Renderer::autoCalculateCellSize() {
  if (!m_grid)
    return;

  sf::Vector2u windowSize = m_window.getSize();
  float maxCellWidth = static_cast<float>(windowSize.x) / m_grid->getWidth();
  float maxCellHeight = static_cast<float>(windowSize.y) / m_grid->getHeight();

  m_cellSize = std::min(maxCellWidth, maxCellHeight) * 0.9f;
  m_cellSize = std::max(m_cellSize, 4.0f); // Minimum cell size

  // Adjust wall thickness based on cell size
  m_wallThickness = std::max(1.0f, m_cellSize / 10.0f);
}

void Renderer::renderCell(int x, int y) {
  // Render cell floor
  sf::Color cellColor = getCellColor(x, y);
  renderCellFloor(x, y, cellColor);

  // Render walls
  renderCellWalls(x, y);
}

void Renderer::renderCellFloor(int x, int y, const sf::Color &color) {
  float px = x * m_cellSize;
  float py = y * m_cellSize;

  m_cellShape.setSize({m_cellSize, m_cellSize});
  m_cellShape.setPosition({px, py});
  m_cellShape.setFillColor(color);
  m_window.draw(m_cellShape);
}

void Renderer::renderCellWalls(int x, int y) {
  float px = x * m_cellSize;
  float py = y * m_cellSize;

  const auto &theme = getTheme();
  m_wallShape.setFillColor(theme.wall);

  uint8_t cell = m_grid->getCell(x, y);

  // North wall
  if (hasFlag(cell, CellFlags::WallNorth)) {
    m_wallShape.setSize({m_cellSize + m_wallThickness, m_wallThickness});
    m_wallShape.setPosition(
        {px - m_wallThickness / 2, py - m_wallThickness / 2});
    m_window.draw(m_wallShape);
  }

  // East wall
  if (hasFlag(cell, CellFlags::WallEast)) {
    m_wallShape.setSize({m_wallThickness, m_cellSize + m_wallThickness});
    m_wallShape.setPosition(
        {px + m_cellSize - m_wallThickness / 2, py - m_wallThickness / 2});
    m_window.draw(m_wallShape);
  }

  // South wall
  if (hasFlag(cell, CellFlags::WallSouth)) {
    m_wallShape.setSize({m_cellSize + m_wallThickness, m_wallThickness});
    m_wallShape.setPosition(
        {px - m_wallThickness / 2, py + m_cellSize - m_wallThickness / 2});
    m_window.draw(m_wallShape);
  }

  // West wall
  if (hasFlag(cell, CellFlags::WallWest)) {
    m_wallShape.setSize({m_wallThickness, m_cellSize + m_wallThickness});
    m_wallShape.setPosition(
        {px - m_wallThickness / 2, py - m_wallThickness / 2});
    m_window.draw(m_wallShape);
  }
}

void Renderer::renderMarkers() {
  if (!m_grid)
    return;

  const auto &theme = getTheme();
  float markerRadius = m_cellSize / 3.0f;
  m_markerShape.setRadius(markerRadius);

  // Start marker
  Position start = m_grid->getStart();
  float startX = start.x * m_cellSize + m_cellSize / 2.0f - markerRadius;
  float startY = start.y * m_cellSize + m_cellSize / 2.0f - markerRadius;
  m_markerShape.setPosition({startX, startY});
  m_markerShape.setFillColor(theme.start);
  m_window.draw(m_markerShape);

  // End marker
  Position end = m_grid->getEnd();
  float endX = end.x * m_cellSize + m_cellSize / 2.0f - markerRadius;
  float endY = end.y * m_cellSize + m_cellSize / 2.0f - markerRadius;
  m_markerShape.setPosition({endX, endY});
  m_markerShape.setFillColor(theme.end);
  m_window.draw(m_markerShape);
}

sf::Color Renderer::getCellColor(int x, int y) const {
  const auto &theme = getTheme();
  uint8_t cell = m_grid->getCell(x, y);

  // Priority: Current > Visited > Queued > Empty
  // InPath is rendered as overlay in renderPath()

  if (hasFlag(cell, CellFlags::Current)) {
    // During generation, use generation color
    // During solving, use algorithm-specific color
    return theme.getCurrentColor(m_algorithmType);
  }

  if (hasFlag(cell, CellFlags::Visited)) {
    return theme.getVisitedColor(m_algorithmType);
  }

  if (hasFlag(cell, CellFlags::Queued)) {
    return theme.getQueuedColor(m_algorithmType);
  }

  return theme.cellEmpty;
}

void Renderer::renderTerrain() {
  if (!m_grid)
    return;

  // Get visible cell range
  Camera::CellRange range = m_camera.getVisibleCells(
      m_cellSize, m_grid->getWidth(), m_grid->getHeight());

  // Render terrain as semi-transparent overlay
  for (int y = range.minY; y <= range.maxY; ++y) {
    for (int x = range.minX; x <= range.maxX; ++x) {
      TerrainType terrain = m_grid->getTerrain(x, y);

      // Skip normal terrain (no overlay needed)
      if (terrain == TerrainType::Normal)
        continue;

      // Don't overlay on visited/path cells
      uint8_t cell = m_grid->getCell(x, y);
      if (hasFlag(cell, CellFlags::InPath) ||
          hasFlag(cell, CellFlags::Visited) ||
          hasFlag(cell, CellFlags::Current)) {
        continue;
      }

      float px = x * m_cellSize;
      float py = y * m_cellSize;

      sf::Color terrainColor = getTerrainColor(terrain);
      terrainColor.a = 180; // Semi-transparent

      m_cellShape.setSize(
          {m_cellSize - m_wallThickness, m_cellSize - m_wallThickness});
      m_cellShape.setPosition(
          {px + m_wallThickness / 2, py + m_wallThickness / 2});
      m_cellShape.setFillColor(terrainColor);
      m_window.draw(m_cellShape);
    }
  }
}

void Renderer::renderPath() {
  if (!m_grid)
    return;

  Camera::CellRange range = m_camera.getVisibleCells(
      m_cellSize, m_grid->getWidth(), m_grid->getHeight());

  const auto &theme = getTheme();
  float pathSize = m_cellSize * 0.35f; // 35% of cell size - thin line
  float offset = (m_cellSize - pathSize) / 2.0f;

  sf::RectangleShape pathRect({pathSize, pathSize});
  pathRect.setFillColor(theme.path);

  for (int y = range.minY; y <= range.maxY; ++y) {
    for (int x = range.minX; x <= range.maxX; ++x) {
      uint8_t cell = m_grid->getCell(x, y);

      if (hasFlag(cell, CellFlags::InPath)) {
        float px = x * m_cellSize;
        float py = y * m_cellSize;

        // Draw center node
        pathRect.setPosition({px + offset, py + offset});
        m_window.draw(pathRect);

        // Draw connecting lines to adjacent path cells ONLY if no wall blocks
        // East connection (check if we can actually move East)
        if (x + 1 < m_grid->getWidth() &&
            hasFlag(m_grid->getCell(x + 1, y), CellFlags::InPath) &&
            m_grid->canMove({x, y}, Direction::East)) {
          sf::RectangleShape conn({m_cellSize - pathSize, pathSize});
          conn.setPosition({px + offset + pathSize, py + offset});
          conn.setFillColor(theme.path);
          m_window.draw(conn);
        }
        // South connection (check if we can actually move South)
        if (y + 1 < m_grid->getHeight() &&
            hasFlag(m_grid->getCell(x, y + 1), CellFlags::InPath) &&
            m_grid->canMove({x, y}, Direction::South)) {
          sf::RectangleShape conn({pathSize, m_cellSize - pathSize});
          conn.setPosition({px + offset, py + offset + pathSize});
          conn.setFillColor(theme.path);
          m_window.draw(conn);
        }
      }
    }
  }
}

void Renderer::renderUI(const std::string &status, int animationDelay) {
  // Reset view to default for UI rendering
  m_window.setView(m_window.getDefaultView());

  // If no font loaded, skip text rendering
  // UI will be text-based in terminal for now
  // This can be enhanced later with SFML font rendering
}

void Renderer::updateFPS() {
  ++m_frameCount;
  float elapsed = m_fpsClock.getElapsedTime().asSeconds();

  if (elapsed >= 1.0f) {
    m_stats.fps = static_cast<float>(m_frameCount) / elapsed;
    m_frameCount = 0;
    m_fpsClock.restart();
  }
}

} // namespace maze
