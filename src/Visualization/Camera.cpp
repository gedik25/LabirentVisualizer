#include "Camera.hpp"
#include <cmath>

namespace maze {

Camera::Camera(float windowWidth, float windowHeight)
    : m_position{windowWidth / 2.0f, windowHeight / 2.0f},
      m_windowSize{windowWidth, windowHeight}, m_zoom{1.0f} {
  m_view.setSize({windowWidth, windowHeight});
  m_view.setCenter(m_position);
}

void Camera::update() {
  // Apply zoom to view size
  sf::Vector2f viewSize = m_windowSize / m_zoom;
  m_view.setSize(viewSize);
  m_view.setCenter(m_position);
}

void Camera::apply(sf::RenderWindow &window) const { window.setView(m_view); }

void Camera::pan(float dx, float dy) {
  // Adjust pan speed based on zoom level
  float adjustedSpeed = panSpeed / m_zoom;
  m_position.x += dx * adjustedSpeed;
  m_position.y += dy * adjustedSpeed;
  update();
}

void Camera::zoom(float factor) { setZoom(m_zoom * factor); }

void Camera::setZoom(float zoom) {
  m_zoom = std::clamp(zoom, MinZoom, MaxZoom);
  update();
}

void Camera::setPosition(float x, float y) {
  m_position = {x, y};
  update();
}

void Camera::setWindowSize(float width, float height) {
  m_windowSize = {width, height};
  update();
}

void Camera::fitToMaze(int mazeWidth, int mazeHeight, float cellSize) {
  // Calculate total maze size in pixels
  float totalWidth = (mazeWidth * 2 + 1) * cellSize;
  float totalHeight = (mazeHeight * 2 + 1) * cellSize;

  // Calculate zoom to fit maze in window with some padding
  float paddingFactor = 0.9f; // 90% of window
  float zoomX = (m_windowSize.x * paddingFactor) / totalWidth;
  float zoomY = (m_windowSize.y * paddingFactor) / totalHeight;

  // Use the smaller zoom to fit both dimensions
  m_zoom = std::min(zoomX, zoomY);
  m_zoom = std::clamp(m_zoom, MinZoom, MaxZoom);

  // Center the maze
  m_position = {totalWidth / 2.0f, totalHeight / 2.0f};

  update();
}

Camera::Bounds Camera::getViewBounds() const {
  sf::Vector2f viewSize = m_view.getSize();
  sf::Vector2f center = m_view.getCenter();

  return {
      center.x - viewSize.x / 2.0f, // left
      center.y - viewSize.y / 2.0f, // top
      center.x + viewSize.x / 2.0f, // right
      center.y + viewSize.y / 2.0f  // bottom
  };
}

Camera::CellRange Camera::getVisibleCells(float cellSize, int gridWidth,
                                          int gridHeight) const {
  Bounds bounds = getViewBounds();

  // Convert pixel bounds to cell indices with some margin
  int minX = static_cast<int>(std::floor(bounds.left / cellSize)) - 1;
  int minY = static_cast<int>(std::floor(bounds.top / cellSize)) - 1;
  int maxX = static_cast<int>(std::ceil(bounds.right / cellSize)) + 1;
  int maxY = static_cast<int>(std::ceil(bounds.bottom / cellSize)) + 1;

  // Clamp to grid bounds
  minX = std::max(0, minX);
  minY = std::max(0, minY);
  maxX = std::min(gridWidth - 1, maxX);
  maxY = std::min(gridHeight - 1, maxY);

  return {minX, minY, maxX, maxY};
}

sf::Vector2f Camera::screenToWorld(const sf::Vector2i &screenPos) const {
  // Convert screen coordinates to world coordinates
  sf::Vector2f viewSize = m_view.getSize();
  sf::Vector2f center = m_view.getCenter();

  float worldX = center.x + (screenPos.x - m_windowSize.x / 2.0f) / m_zoom;
  float worldY = center.y + (screenPos.y - m_windowSize.y / 2.0f) / m_zoom;

  return {worldX, worldY};
}

} // namespace maze
