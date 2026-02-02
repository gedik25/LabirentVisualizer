#include "UIPanel.hpp"
#include <iomanip>
#include <sstream>

namespace maze {

UIPanel::UIPanel() = default;

bool UIPanel::init(sf::RenderWindow &window) {
  // Try to load font from various locations
  std::vector<std::string> fontPaths = {
      // macOS system fonts
      "/System/Library/Fonts/SFNSMono.ttf", "/System/Library/Fonts/Menlo.ttc",
      "/System/Library/Fonts/Monaco.ttf", "/Library/Fonts/Arial.ttf",
      // Linux common fonts
      "/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf",
      "/usr/share/fonts/TTF/DejaVuSansMono.ttf",
      // Windows fonts
      "C:/Windows/Fonts/consola.ttf", "C:/Windows/Fonts/arial.ttf"};

  m_font.emplace();
  for (const auto &path : fontPaths) {
    if (m_font->openFromFile(path)) {
      m_fontLoaded = true;
      return true;
    }
  }

  m_font.reset();
  m_fontLoaded = false;
  return false;
}

void UIPanel::render(sf::RenderWindow &window) {
  if (!m_visible || !m_fontLoaded)
    return;

  // Save current view and switch to default view for UI rendering
  sf::View currentView = window.getView();
  window.setView(window.getDefaultView());

  // Render terrain banner if terrain is enabled
  if (m_terrainEnabled) {
    renderTerrainBanner(window);
  }

  // Render panels
  renderStatusPanel(window);
  renderStatsPanel(window);

  if (m_showHelp) {
    renderHelpPanel(window);
  }

  // Restore camera view
  window.setView(currentView);
}

void UIPanel::setGridSize(int width, int height) {
  m_gridWidth = width;
  m_gridHeight = height;
}

void UIPanel::setState(const std::string &state) { m_state = state; }

void UIPanel::setAnimationEnabled(bool enabled) {
  m_animationEnabled = enabled;
}

void UIPanel::setAnimationDelay(int delayMs) { m_animationDelay = delayMs; }

void UIPanel::setFPS(float fps) { m_fps = fps; }

void UIPanel::setAlgorithmName(const std::string &name) {
  m_algorithmName = name;
}

void UIPanel::setNodesVisited(size_t count) { m_nodesVisited = count; }

void UIPanel::setQueueSize(size_t size) { m_queueSize = size; }

void UIPanel::setPathLength(size_t length) { m_pathLength = length; }

void UIPanel::setElapsedTime(float timeMs) { m_elapsedTime = timeMs; }

void UIPanel::setGeneratorName(const std::string &name) {
  m_generatorName = name;
}

void UIPanel::setDifficultyName(const std::string &name) {
  m_difficultyName = name;
}

void UIPanel::setTerrainEnabled(bool enabled) { m_terrainEnabled = enabled; }

void UIPanel::setPathCost(float cost) { m_pathCost = cost; }

void UIPanel::toggleHelp() { m_showHelp = !m_showHelp; }

void UIPanel::renderStatusPanel(sf::RenderWindow &window) {
  float panelWidth = 220.0f;
  float panelHeight = 195.0f;
  float x = m_padding;
  float y = m_padding;

  // Background
  renderPanel(window, x, y, panelWidth, panelHeight);

  // Content
  float textX = x + m_padding;
  float textY = y + m_padding;

  // Title
  renderText(window, "STATUS", textX, textY, m_headerColor);
  textY += m_lineHeight + 5;

  // Grid size
  std::stringstream ss;
  ss << m_gridWidth << " x " << m_gridHeight;
  renderTextPair(window, "Grid:", ss.str(), textX, textY);
  textY += m_lineHeight;

  // State
  renderTextPair(window, "State:", m_state, textX, textY);
  textY += m_lineHeight;

  // Generator
  renderTextPair(window, "Gen:", m_generatorName, textX, textY);
  textY += m_lineHeight;

  // Difficulty
  renderTextPair(window, "Diff:", m_difficultyName, textX, textY);
  textY += m_lineHeight;

  // Terrain
  renderTextPair(window, "Terrain:", m_terrainEnabled ? "ON" : "OFF", textX,
                 textY);
  textY += m_lineHeight;

  // Animation
  renderTextPair(window, "Anim:", m_animationEnabled ? "ON" : "OFF", textX,
                 textY);
  textY += m_lineHeight;

  // Delay
  ss.str("");
  ss << m_animationDelay << " ms";
  renderTextPair(window, "Delay:", ss.str(), textX, textY);
  textY += m_lineHeight;

  // FPS
  ss.str("");
  ss << std::fixed << std::setprecision(0) << m_fps;
  renderTextPair(window, "FPS:", ss.str(), textX, textY);
}

void UIPanel::renderStatsPanel(sf::RenderWindow &window) {
  if (m_algorithmName.empty())
    return;

  float panelWidth = 220.0f;
  float panelHeight = 150.0f; // Increased for path cost
  float x = m_padding;
  float y = m_padding + 205.0f; // Below STATUS panel (195 + 10 padding)

  // Background
  renderPanel(window, x, y, panelWidth, panelHeight);

  // Content
  float textX = x + m_padding;
  float textY = y + m_padding;

  // Title (algorithm name)
  std::string shortName = m_algorithmName;
  if (shortName.length() > 15) {
    shortName = shortName.substr(0, 3); // Just "BFS" or "DFS"
  }
  renderText(window, shortName, textX, textY, m_headerColor);
  textY += m_lineHeight + 5;

  // Visited
  std::stringstream ss;
  ss << m_nodesVisited;
  renderTextPair(window, "Visited:", ss.str(), textX, textY);
  textY += m_lineHeight;

  // Queue/Stack size
  ss.str("");
  ss << m_queueSize;
  renderTextPair(window, "Queue:", ss.str(), textX, textY);
  textY += m_lineHeight;

  // Path length
  ss.str("");
  ss << m_pathLength;
  renderTextPair(window, "Path:", ss.str(), textX, textY);
  textY += m_lineHeight;

  // Time
  ss.str("");
  ss << std::fixed << std::setprecision(1) << m_elapsedTime << " ms";
  renderTextPair(window, "Time:", ss.str(), textX, textY);
  textY += m_lineHeight;

  // Path cost (if terrain enabled and path found)
  if (m_terrainEnabled && m_pathCost > 0) {
    ss.str("");
    ss << std::fixed << std::setprecision(1) << m_pathCost;
    renderTextPair(window, "Cost:", ss.str(), textX, textY);
  }
}

void UIPanel::renderHelpPanel(sf::RenderWindow &window) {
  sf::Vector2u windowSize = window.getSize();

  float panelWidth = 280.0f;
  float panelHeight = 580.0f;
  float x = windowSize.x - panelWidth - m_padding;
  float y = m_padding;

  // Background
  renderPanel(window, x, y, panelWidth, panelHeight);

  // Content
  float textX = x + m_padding;
  float textY = y + m_padding;
  float keyWidth = 70.0f;

  // Title
  renderText(window, "CONTROLS", textX, textY, m_headerColor);
  textY += m_lineHeight + 10;

  // --- Generation ---
  renderText(window, "-- Generation --", textX, textY,
             sf::Color(100, 100, 100));
  textY += m_lineHeight;

  std::vector<std::pair<std::string, std::string>> genControls = {
      {"G", "Generate (animated)"},       {"Enter", "Generate (instant)"},
      {"Shift+1", "Recursive Backtrack"}, {"Shift+2", "Prim's Algorithm"},
      {"Shift+3", "Kruskal's Algorithm"}, {"Shift+4", "Binary Tree"},
      {"Shift+5", "Eller's Algorithm"}};

  for (const auto &[key, desc] : genControls) {
    renderText(window, key, textX, textY, m_highlightColor);
    renderText(window, desc, textX + keyWidth, textY, m_textColor);
    textY += m_lineHeight;
  }

  textY += 5;

  // --- Pathfinding ---
  renderText(window, "-- Pathfinding --", textX, textY,
             sf::Color(100, 100, 100));
  textY += m_lineHeight;

  std::vector<std::pair<std::string, std::string>> solveControls = {
      {"B", "BFS (Breadth-First)"}, {"D", "DFS (Depth-First)"},
      {"A", "A* (A-Star)"},         {"J", "Dijkstra"},
      {"Y", "Greedy Best-First"},   {"I", "Bidirectional BFS"}};

  for (const auto &[key, desc] : solveControls) {
    renderText(window, key, textX, textY, m_highlightColor);
    renderText(window, desc, textX + keyWidth, textY, m_textColor);
    textY += m_lineHeight;
  }

  textY += 5;

  // --- Options ---
  renderText(window, "-- Options --", textX, textY, sf::Color(100, 100, 100));
  textY += m_lineHeight;

  std::vector<std::pair<std::string, std::string>> optControls = {
      {"T", "Toggle terrain"},
      {"M", "Open menu"},
      {"Space", "Toggle animation"},
      {"+/-", "Adjust speed"},
      {"1/2/3", "Size: 25/100/500"}};

  for (const auto &[key, desc] : optControls) {
    renderText(window, key, textX, textY, m_highlightColor);
    renderText(window, desc, textX + keyWidth, textY, m_textColor);
    textY += m_lineHeight;
  }

  textY += 5;

  // --- Navigation ---
  renderText(window, "-- Navigation --", textX, textY,
             sf::Color(100, 100, 100));
  textY += m_lineHeight;

  std::vector<std::pair<std::string, std::string>> navControls = {
      {"WASD", "Pan camera"}, {"Scroll", "Zoom in/out"},
      {"F", "Fit to window"}, {"R", "Reset solution"},
      {"C", "Clear all"},     {"H", "Toggle help"},
      {"ESC", "Exit"}};

  for (const auto &[key, desc] : navControls) {
    renderText(window, key, textX, textY, m_highlightColor);
    renderText(window, desc, textX + keyWidth, textY, m_textColor);
    textY += m_lineHeight;
  }
}

void UIPanel::renderPanel(sf::RenderWindow &window, float x, float y,
                          float width, float height) {
  sf::RectangleShape panel({width, height});
  panel.setPosition({x, y});
  panel.setFillColor(m_bgColor);
  panel.setOutlineColor(sf::Color(60, 60, 60));
  panel.setOutlineThickness(1.0f);
  window.draw(panel);
}

void UIPanel::renderText(sf::RenderWindow &window, const std::string &text,
                         float x, float y, const sf::Color &color) {
  if (!m_fontLoaded)
    return;

  sf::Text sfText(*m_font, text, m_fontSize);
  sfText.setPosition({x, y});
  sfText.setFillColor(color);
  window.draw(sfText);
}

void UIPanel::renderTextPair(sf::RenderWindow &window, const std::string &label,
                             const std::string &value, float x, float y) {
  renderText(window, label, x, y, m_textColor);
  renderText(window, value, x + 75.0f, y, m_highlightColor);
}

void UIPanel::renderTerrainBanner(sf::RenderWindow &window) {
  if (!m_fontLoaded)
    return;

  sf::Vector2u windowSize = window.getSize();

  float bannerWidth = 450.0f;
  float bannerHeight = 50.0f;
  float x = (windowSize.x - bannerWidth) / 2.0f;
  float y = 10.0f;

  // Background
  sf::RectangleShape bg({bannerWidth, bannerHeight});
  bg.setPosition({x, y});
  bg.setFillColor(sf::Color(0, 80, 40, 220));
  bg.setOutlineColor(m_terrainColor);
  bg.setOutlineThickness(2.0f);
  window.draw(bg);

  // Title
  sf::Text title(*m_font, "TERRAIN MODE ACTIVE", 16);
  title.setPosition({x + 15.0f, y + 8.0f});
  title.setFillColor(m_terrainColor);
  window.draw(title);

  // Subtitle with path cost if available
  std::stringstream ss;
  if (m_pathCost > 0) {
    ss << "Use Dijkstra/A* for weights | Path Cost: " << std::fixed
       << std::setprecision(1) << m_pathCost;
  } else {
    ss << "Weights: Grass(1), Water(5), Sand(10) | Use Dijkstra/A*";
  }
  sf::Text subtitle(*m_font, ss.str(), 12);
  subtitle.setPosition({x + 15.0f, y + 28.0f});
  subtitle.setFillColor(sf::Color(200, 255, 200));
  window.draw(subtitle);
}

} // namespace maze
