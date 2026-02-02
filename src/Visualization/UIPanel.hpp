#pragma once

#include <SFML/Graphics.hpp>
#include <optional>
#include <string>
#include <vector>

namespace maze {

/**
 * UI Panel for displaying status, statistics, and controls.
 * Renders text overlays on the maze visualization.
 */
class UIPanel {
public:
  UIPanel();

  // Initialize with window reference
  bool init(sf::RenderWindow &window);

  // Render all UI elements
  void render(sf::RenderWindow &window);

  // Update status information
  void setGridSize(int width, int height);
  void setState(const std::string &state);
  void setAnimationEnabled(bool enabled);
  void setAnimationDelay(int delayMs);
  void setFPS(float fps);

  // New: Generator, Difficulty, Terrain info
  void setGeneratorName(const std::string &name);
  void setDifficultyName(const std::string &name);
  void setTerrainEnabled(bool enabled);
  void setPathCost(float cost);

  // Update algorithm statistics
  void setAlgorithmName(const std::string &name);
  void setNodesVisited(size_t count);
  void setQueueSize(size_t size);
  void setPathLength(size_t length);
  void setElapsedTime(float timeMs);

  // Toggle help panel visibility
  void toggleHelp();
  bool isHelpVisible() const { return m_showHelp; }

  // Panel visibility
  void setVisible(bool visible) { m_visible = visible; }
  bool isVisible() const { return m_visible; }

private:
  std::optional<sf::Font> m_font;
  bool m_fontLoaded = false;
  bool m_visible = true;
  bool m_showHelp = true;

  // Status info
  int m_gridWidth = 0;
  int m_gridHeight = 0;
  std::string m_state = "Idle";
  bool m_animationEnabled = true;
  int m_animationDelay = 10;
  float m_fps = 0.0f;

  // New: Generator, Difficulty, Terrain info
  std::string m_generatorName = "Recursive";
  std::string m_difficultyName = "Medium";
  bool m_terrainEnabled = false;
  float m_pathCost = 0.0f;

  // Algorithm stats
  std::string m_algorithmName = "";
  size_t m_nodesVisited = 0;
  size_t m_queueSize = 0;
  size_t m_pathLength = 0;
  float m_elapsedTime = 0.0f;

  // UI colors
  sf::Color m_bgColor{0, 0, 0, 180};       // Semi-transparent black
  sf::Color m_textColor{255, 255, 255};    // White
  sf::Color m_highlightColor{0, 229, 255}; // Cyan
  sf::Color m_headerColor{255, 214, 0};    // Yellow
  sf::Color m_terrainColor{0, 255, 127};   // Green for terrain banner

  // Font size
  unsigned int m_fontSize = 14;
  float m_padding = 10.0f;
  float m_lineHeight = 20.0f;

  // Render helpers
  void renderStatusPanel(sf::RenderWindow &window);
  void renderStatsPanel(sf::RenderWindow &window);
  void renderHelpPanel(sf::RenderWindow &window);
  void renderTerrainBanner(sf::RenderWindow &window);
  void renderPanel(sf::RenderWindow &window, float x, float y, float width,
                   float height);
  void renderText(sf::RenderWindow &window, const std::string &text, float x,
                  float y, const sf::Color &color = sf::Color::White);
  void renderTextPair(sf::RenderWindow &window, const std::string &label,
                      const std::string &value, float x, float y);
};

} // namespace maze
