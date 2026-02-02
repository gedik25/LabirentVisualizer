#pragma once

#include "../Algorithms/ISolver.hpp"
#include "../Core/Grid.hpp"
#include <SFML/Graphics.hpp>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace maze {

/**
 * Menu state for comparison mode
 */
enum class ComparisonMenuState {
  Hidden,              // Menu not visible
  SelectingCount,      // Selecting 2 or 4 algorithms
  SelectingAlgorithms, // Selecting which algorithms
  Running              // Comparison running, split screen active
};

/**
 * Comparison view for running multiple algorithms on the same maze.
 * Supports 2 or 4 algorithms side-by-side with a selection menu.
 */
class ComparisonView {
public:
  ComparisonView();

  // Initialize with window reference
  bool init(sf::RenderWindow &window);

  // Handle keyboard input, returns true if input was consumed
  bool handleInput(sf::Keyboard::Key key);

  // Open the comparison menu
  void openMenu();

  // Close menu and return to normal mode
  void closeMenu();

  // Start comparison after selection
  void startComparison();

  // Set the number of algorithms to compare (2 or 4)
  void setAlgorithmCount(int count);
  int getAlgorithmCount() const { return m_algorithmCount; }

  // Set which algorithms to compare (by index)
  // 0=BFS, 1=DFS, 2=A*, 3=Dijkstra, 4=Greedy, 5=Bidirectional
  void setAlgorithm(int slot, int algorithmIndex);
  int getAlgorithm(int slot) const;

  // Set the shared grid (all algorithms use the same maze)
  void setGrid(std::shared_ptr<Grid> grid);

  // Initialize solvers with start/end positions
  void initSolvers(const Position &start, const Position &end);

  // Step all solvers simultaneously
  bool stepAll();

  // Check if all solvers are finished
  bool allFinished() const;

  // Reset all solvers
  void resetSolvers();

  // Get solver by slot
  ISolver *getSolver(int slot);

  // Render the comparison view (menu or split screen)
  void render(sf::RenderWindow &window);

  // Render results table
  void renderResults(sf::RenderWindow &window);

  // Visibility and state
  void show() { m_visible = true; }
  void hide() {
    m_visible = false;
    m_menuState = ComparisonMenuState::Hidden;
  }
  bool isVisible() const { return m_visible; }
  bool isMenuOpen() const {
    return m_menuState != ComparisonMenuState::Hidden &&
           m_menuState != ComparisonMenuState::Running;
  }
  bool isRunning() const { return m_menuState == ComparisonMenuState::Running; }
  ComparisonMenuState getMenuState() const { return m_menuState; }

  // Get algorithm name by index
  static std::string getAlgorithmName(int index);
  static int getTotalAlgorithms() { return 6; } // Total available algorithms

private:
  bool m_visible = false;
  std::optional<sf::Font> m_font;
  bool m_fontLoaded = false;

  // Menu state
  ComparisonMenuState m_menuState = ComparisonMenuState::Hidden;
  int m_menuSelection = 0;               // Current menu item selection
  int m_algorithmCount = 2;              // Number of algorithms to compare
  std::vector<int> m_selectedAlgorithms; // Algorithm indices for each slot
  std::shared_ptr<Grid> m_grid;

  // Create solver by index
  std::unique_ptr<ISolver> createSolver(int index, std::shared_ptr<Grid> grid);

  // Solver instances for each slot
  std::vector<std::unique_ptr<ISolver>> m_solvers;

  // Grids for each slot (copies for independent visualization)
  std::vector<std::shared_ptr<Grid>> m_grids;

  // Layout calculations
  struct ViewportRect {
    float x, y, width, height;
  };
  std::vector<ViewportRect> m_viewports;

  void calculateViewports(sf::RenderWindow &window);

  // Render helpers
  void renderMenu(sf::RenderWindow &window);
  void renderViewport(sf::RenderWindow &window, int slot,
                      const ViewportRect &vp);
  void renderMiniGrid(sf::RenderWindow &window, Grid &grid,
                      const ViewportRect &vp, const sf::Color &visitedColor);
  void renderText(sf::RenderWindow &window, const std::string &text, float x,
                  float y, const sf::Color &color, unsigned int size = 14);
  void renderStatsPanel(sf::RenderWindow &window);

  // Colors for each slot
  static const std::array<sf::Color, 4> s_slotColors;
};

} // namespace maze
