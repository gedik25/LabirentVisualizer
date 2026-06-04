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

  int w = m_grid->getWidth();
  int h = m_grid->getHeight();
  int tilesX = w * 2 + 1;
  int tilesY = h * 2 + 1;

  // We could use spatial culling here, but for now drawn entirely.
  // With tileSize, total size is tilesX * m_cellSize

  sf::Color wallColor = sf::Color::Black;

  auto getCellColor = [&](int cx, int cy) -> sf::Color {
    uint8_t flags = m_grid->getCell(cx, cy);
    TerrainType terrain = m_grid->getTerrain(cx, cy);
    sf::Color color = getTerrainColor(terrain);

    if (hasFlag(flags, CellFlags::InPath)) {
      color = sf::Color{255, 0, 255}; // Path magenta
    } else if (hasFlag(flags, CellFlags::Current)) {
      color = sf::Color{255, 255, 0}; // Yellow
    } else if (hasFlag(flags, CellFlags::Visited)) {
      if (m_heatmapEnabled && m_maxVisited > 0) {
        ++m_visitCounter;
        float t = static_cast<float>(m_visitCounter) /
                  static_cast<float>(m_maxVisited);
        color = heatmapColor(t);
      } else {
        color = getTheme().getVisitedColor(m_algorithmType);
      }
    } else if (hasFlag(flags, CellFlags::Queued)) {
      color = getTheme().getQueuedColor(m_algorithmType);
    }
    return color;
  };

  auto getPassageColor = [&](int c1x, int c1y, int c2x, int c2y) -> sf::Color {
    uint8_t f1 = m_grid->getCell(c1x, c1y);
    uint8_t f2 = m_grid->getCell(c2x, c2y);

    if (hasFlag(f1, CellFlags::InPath) && hasFlag(f2, CellFlags::InPath))
      return sf::Color{255, 0, 255};
    if (hasFlag(f1, CellFlags::Visited) && hasFlag(f2, CellFlags::Visited)) {
      if (m_heatmapEnabled && m_maxVisited > 0) {
        ++m_visitCounter;
        float t = static_cast<float>(m_visitCounter) /
                  static_cast<float>(m_maxVisited);
        return heatmapColor(t);
      }
      return getTheme().getVisitedColor(m_algorithmType);
    }
    if (hasFlag(f1, CellFlags::Queued) && hasFlag(f2, CellFlags::Queued))
      return getTheme().getQueuedColor(m_algorithmType);

    TerrainType t1 = m_grid->getTerrain(c1x, c1y);
    TerrainType t2 = m_grid->getTerrain(c2x, c2y);
    TerrainType terrain = (t1 != TerrainType::Normal) ? t1 : t2;
    return getTerrainColor(terrain);
  };

  m_cellShape.setSize({m_cellSize, m_cellSize});

  m_visitCounter = 0; // Reset heatmap visit counter each frame
  m_stats.cellsRendered = 0;

  for (int ty = 0; ty < tilesY; ++ty) {
    for (int tx = 0; tx < tilesX; ++tx) {
      float px = tx * m_cellSize;
      float py = ty * m_cellSize;
      m_cellShape.setPosition({px, py});

      bool isWallX = (tx % 2 == 0);
      bool isWallY = (ty % 2 == 0);

      if (isWallX && isWallY) {
        m_cellShape.setFillColor(wallColor);
      } else if (!isWallX && !isWallY) {
        int cx = (tx - 1) / 2;
        int cy = (ty - 1) / 2;
        m_cellShape.setFillColor(getCellColor(cx, cy));
      } else if (isWallX && !isWallY) {
        int cx = tx / 2;
        int cy = (ty - 1) / 2;
        if (cx == 0 || cx == w) {
          m_cellShape.setFillColor(wallColor);
        } else {
          if (hasFlag(m_grid->getCell(cx, cy), CellFlags::WallWest)) {
            m_cellShape.setFillColor(wallColor);
          } else {
            m_cellShape.setFillColor(getPassageColor(cx - 1, cy, cx, cy));
          }
        }
      } else if (!isWallX && isWallY) {
        int cx = (tx - 1) / 2;
        int cy = ty / 2;
        if (cy == 0 || cy == h) {
          m_cellShape.setFillColor(wallColor);
        } else {
          if (hasFlag(m_grid->getCell(cx, cy), CellFlags::WallNorth)) {
            m_cellShape.setFillColor(wallColor);
          } else {
            m_cellShape.setFillColor(getPassageColor(cx, cy - 1, cx, cy));
          }
        }
      }

      m_window.draw(m_cellShape);
      ++m_stats.cellsRendered;
    }
  }

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
  float gridTilesX = m_grid->getWidth() * 2 + 1.0f;
  float gridTilesY = m_grid->getHeight() * 2 + 1.0f;
  float maxCellWidth = static_cast<float>(windowSize.x) / gridTilesX;
  float maxCellHeight = static_cast<float>(windowSize.y) / gridTilesY;

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
  float startX =
      (start.x * 2 + 1) * m_cellSize + m_cellSize / 2.0f - markerRadius;
  float startY =
      (start.y * 2 + 1) * m_cellSize + m_cellSize / 2.0f - markerRadius;
  m_markerShape.setPosition({startX, startY});
  m_markerShape.setFillColor(theme.start);
  m_window.draw(m_markerShape);

  // End marker
  Position end = m_grid->getEnd();
  float endX = (end.x * 2 + 1) * m_cellSize + m_cellSize / 2.0f - markerRadius;
  float endY = (end.y * 2 + 1) * m_cellSize + m_cellSize / 2.0f - markerRadius;
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

sf::Color Renderer::heatmapColor(float t) const {
  // Clamp t to [0, 1]
  if (t < 0.0f)
    t = 0.0f;
  if (t > 1.0f)
    t = 1.0f;

  // 5-stop gradient: Blue -> Cyan -> Green -> Yellow -> Red
  uint8_t r, g, b;
  if (t < 0.25f) {
    float s = t / 0.25f;
    r = 0;
    g = static_cast<uint8_t>(255 * s);
    b = 255;
  } else if (t < 0.5f) {
    float s = (t - 0.25f) / 0.25f;
    r = 0;
    g = 255;
    b = static_cast<uint8_t>(255 * (1.0f - s));
  } else if (t < 0.75f) {
    float s = (t - 0.5f) / 0.25f;
    r = static_cast<uint8_t>(255 * s);
    g = 255;
    b = 0;
  } else {
    float s = (t - 0.75f) / 0.25f;
    r = 255;
    g = static_cast<uint8_t>(255 * (1.0f - s));
    b = 0;
  }
  return sf::Color{r, g, b};
}

} // namespace maze
