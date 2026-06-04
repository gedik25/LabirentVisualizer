#pragma once

#include <SFML/Graphics.hpp>
#include <functional>
#include <optional>
#include <string>
#include <vector>

namespace maze {

/**
 * Interactive menu system with keyboard navigation.
 * Supports sections, items, and callbacks.
 */
class MenuSystem {
public:
  MenuSystem();

  // Initialize with window for font loading
  bool init(sf::RenderWindow &window);

  // Menu visibility
  void show();
  void hide();
  void toggle();
  bool isVisible() const { return m_visible; }

  // Handle keyboard input
  // Returns true if input was consumed
  bool handleKeyPress(sf::Keyboard::Key key);

  // Render the menu
  void render(sf::RenderWindow &window);

  // Menu item types
  enum class ItemType {
    Action,   // Triggers callback when selected
    Selector, // Cycles through options
    Submenu,  // Opens submenu
    Back,     // Returns to parent menu
    Separator // Visual separator
  };

  // Menu item structure
  struct MenuItem {
    std::string label;
    ItemType type;
    std::vector<std::string> options; // For selector type
    int selectedOption = 0;
    std::function<void()> callback;
    std::function<void(int)> onOptionChange;
  };

  // Menu section
  struct MenuSection {
    std::string title;
    std::vector<MenuItem> items;
  };

  // Add menu sections
  void addSection(const MenuSection &section);
  void clearSections();

  // Get current selections
  int getSelectedGenerator() const { return m_selectedGenerator; }
  int getSelectedSolver() const { return m_selectedSolver; }
  int getDifficulty() const { return m_difficulty; }
  int getGridWidth() const { return m_gridWidth; }
  int getGridHeight() const { return m_gridHeight; }

  // Set current selections
  void setSelectedGenerator(int idx) { m_selectedGenerator = idx; }
  void setSelectedSolver(int idx) { m_selectedSolver = idx; }
  void setDifficulty(int diff) { m_difficulty = diff; }
  void setGridSize(int width, int height) {
    m_gridWidth = width;
    m_gridHeight = height;
  }

  // Callbacks for when selections change
  std::function<void(int)> onGeneratorChange;
  std::function<void(int)> onSolverChange;
  std::function<void(int)> onDifficultyChange;
  std::function<void(int, int)> onSizeChange;
  std::function<void()> onApply;
  std::function<void()> onSaveGrid;
  std::function<void()> onLoadGrid;

private:
  bool m_visible = false;
  std::optional<sf::Font> m_font;
  bool m_fontLoaded = false;

  std::vector<MenuSection> m_sections;
  int m_currentSection = 0;
  int m_currentItem = 0;

  // Current selections
  int m_selectedGenerator =
      0; // 0=RecursiveBacktracking, 1=Prim, 2=Kruskal, 3=BinaryTree, 4=Ellers
  int m_selectedSolver =
      0; // 0=BFS, 1=DFS, 2=A*, 3=Dijkstra, 4=Greedy, 5=Bidirectional
  int m_difficulty = 3; // 1-5
  int m_gridWidth = 25;
  int m_gridHeight = 25;

  // Size presets
  struct SizePreset {
    std::string name;
    int width;
    int height;
  };
  std::vector<SizePreset> m_sizePresets;
  int m_currentSizePreset = 0;

  // Visual settings
  float m_menuWidth = 400.0f;
  float m_menuHeight = 500.0f;
  float m_padding = 15.0f;
  float m_itemHeight = 28.0f;
  float m_sectionSpacing = 20.0f;
  unsigned int m_fontSize = 16;
  unsigned int m_titleFontSize = 20;

  // Colors
  sf::Color m_bgColor{20, 20, 20, 240};
  sf::Color m_borderColor{60, 60, 60};
  sf::Color m_titleColor{255, 214, 0};    // Yellow
  sf::Color m_textColor{255, 255, 255};   // White
  sf::Color m_selectedColor{0, 229, 255}; // Cyan
  sf::Color m_valueColor{0, 255, 127};    // Green
  sf::Color m_separatorColor{60, 60, 60};

  // Render helpers
  void renderBackground(sf::RenderWindow &window);
  void renderSection(sf::RenderWindow &window, const MenuSection &section,
                     float &y, int sectionIdx);
  void renderText(sf::RenderWindow &window, const std::string &text, float x,
                  float y, const sf::Color &color, unsigned int size = 0);

  // Navigation
  void moveUp();
  void moveDown();
  void selectItem();
  void cycleOption(int direction);

  // Get total item count for navigation
  int getTotalItems() const;
  void getItemPosition(int flatIndex, int &section, int &item) const;
  int getFlatIndex(int section, int item) const;
};

} // namespace maze
