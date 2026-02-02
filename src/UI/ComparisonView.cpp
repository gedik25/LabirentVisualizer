#include "ComparisonView.hpp"
#include "../Algorithms/AStarSolver.hpp"
#include "../Algorithms/BFSSolver.hpp"
#include "../Algorithms/BidirectionalBFS.hpp"
#include "../Algorithms/DFSSolver.hpp"
#include "../Algorithms/DijkstraSolver.hpp"
#include "../Algorithms/GreedySolver.hpp"
#include <iomanip>
#include <sstream>

namespace maze {

const std::array<sf::Color, 4> ComparisonView::s_slotColors = {{
    sf::Color{0, 229, 255}, // Cyan (BFS)
    sf::Color{255, 214, 0}, // Yellow (DFS)
    sf::Color{0, 255, 127}, // Spring Green (A*)
    sf::Color{255, 165, 0}  // Orange (Dijkstra)
}};

ComparisonView::ComparisonView() {
  m_selectedAlgorithms = {0, 1, 2, 3}; // Default: BFS, DFS, A*, Dijkstra
}

bool ComparisonView::init(sf::RenderWindow &window) {
  // Load font
  std::vector<std::string> fontPaths = {
      "/System/Library/Fonts/SFNSMono.ttf", "/System/Library/Fonts/Menlo.ttc",
      "/System/Library/Fonts/Monaco.ttf",
      "/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf",
      "C:/Windows/Fonts/consola.ttf"};

  m_font.emplace();
  for (const auto &path : fontPaths) {
    if (m_font->openFromFile(path)) {
      m_fontLoaded = true;
      break;
    }
  }

  if (!m_fontLoaded) {
    m_font.reset();
  }

  return m_fontLoaded;
}

void ComparisonView::openMenu() {
  m_visible = true;
  m_menuState = ComparisonMenuState::SelectingCount;
  m_menuSelection = 0;
}

void ComparisonView::closeMenu() {
  m_visible = false;
  m_menuState = ComparisonMenuState::Hidden;
  m_solvers.clear();
  m_grids.clear();
}

void ComparisonView::startComparison() {
  m_menuState = ComparisonMenuState::Running;
  // initSolvers will be called from main.cpp with start/end positions
}

bool ComparisonView::handleInput(sf::Keyboard::Key key) {
  if (!m_visible)
    return false;

  switch (m_menuState) {
  case ComparisonMenuState::SelectingCount:
    if (key == sf::Keyboard::Key::Num2 || key == sf::Keyboard::Key::Numpad2) {
      m_algorithmCount = 2;
      m_menuState = ComparisonMenuState::SelectingAlgorithms;
      m_menuSelection = 0;
      return true;
    }
    if (key == sf::Keyboard::Key::Num4 || key == sf::Keyboard::Key::Numpad4) {
      m_algorithmCount = 4;
      m_menuState = ComparisonMenuState::SelectingAlgorithms;
      m_menuSelection = 0;
      return true;
    }
    if (key == sf::Keyboard::Key::Escape) {
      closeMenu();
      return true;
    }
    break;

  case ComparisonMenuState::SelectingAlgorithms:
    if (key == sf::Keyboard::Key::Up) {
      m_menuSelection =
          (m_menuSelection - 1 + m_algorithmCount) % m_algorithmCount;
      return true;
    }
    if (key == sf::Keyboard::Key::Down) {
      m_menuSelection = (m_menuSelection + 1) % m_algorithmCount;
      return true;
    }
    if (key == sf::Keyboard::Key::Left) {
      int &algo = m_selectedAlgorithms[m_menuSelection];
      algo = (algo - 1 + getTotalAlgorithms()) % getTotalAlgorithms();
      return true;
    }
    if (key == sf::Keyboard::Key::Right) {
      int &algo = m_selectedAlgorithms[m_menuSelection];
      algo = (algo + 1) % getTotalAlgorithms();
      return true;
    }
    if (key == sf::Keyboard::Key::Enter) {
      startComparison();
      return true;
    }
    if (key == sf::Keyboard::Key::Escape ||
        key == sf::Keyboard::Key::Backspace) {
      m_menuState = ComparisonMenuState::SelectingCount;
      return true;
    }
    break;

  case ComparisonMenuState::Running:
    if (key == sf::Keyboard::Key::Escape || key == sf::Keyboard::Key::X) {
      closeMenu();
      return true;
    }
    break;

  default:
    break;
  }

  return false;
}

void ComparisonView::setAlgorithmCount(int count) {
  m_algorithmCount = std::max(2, std::min(4, count));
}

void ComparisonView::setAlgorithm(int slot, int algorithmIndex) {
  if (slot >= 0 && slot < static_cast<int>(m_selectedAlgorithms.size())) {
    m_selectedAlgorithms[slot] = algorithmIndex;
  }
}

int ComparisonView::getAlgorithm(int slot) const {
  if (slot >= 0 && slot < static_cast<int>(m_selectedAlgorithms.size())) {
    return m_selectedAlgorithms[slot];
  }
  return 0;
}

void ComparisonView::setGrid(std::shared_ptr<Grid> grid) { m_grid = grid; }

std::string ComparisonView::getAlgorithmName(int index) {
  switch (index) {
  case 0:
    return "BFS";
  case 1:
    return "DFS";
  case 2:
    return "A*";
  case 3:
    return "Dijkstra";
  case 4:
    return "Greedy";
  case 5:
    return "BiDir BFS";
  default:
    return "Unknown";
  }
}

std::unique_ptr<ISolver>
ComparisonView::createSolver(int index, std::shared_ptr<Grid> grid) {
  switch (index) {
  case 0:
    return std::make_unique<BFSSolver>(grid);
  case 1:
    return std::make_unique<DFSSolver>(grid);
  case 2:
    return std::make_unique<AStarSolver>(grid);
  case 3:
    return std::make_unique<DijkstraSolver>(grid);
  case 4:
    return std::make_unique<GreedySolver>(grid);
  case 5:
    return std::make_unique<BidirectionalBFS>(grid);
  default:
    return std::make_unique<BFSSolver>(grid);
  }
}

void ComparisonView::initSolvers(const Position &start, const Position &end) {
  m_solvers.clear();
  m_grids.clear();

  if (!m_grid)
    return;

  // Create copies of the grid for each solver
  for (int i = 0; i < m_algorithmCount; ++i) {
    // Create a copy of the grid
    auto gridCopy =
        std::make_shared<Grid>(m_grid->getWidth(), m_grid->getHeight());

    // Copy cell data
    for (int y = 0; y < m_grid->getHeight(); ++y) {
      for (int x = 0; x < m_grid->getWidth(); ++x) {
        gridCopy->setCell(x, y, m_grid->getCell(x, y));
        gridCopy->setTerrain(x, y, m_grid->getTerrain(x, y));
      }
    }
    gridCopy->setStart(start);
    gridCopy->setEnd(end);

    m_grids.push_back(gridCopy);

    // Create solver for this slot
    int algoIdx = m_selectedAlgorithms[i];
    auto solver = createSolver(algoIdx, gridCopy);
    solver->init(start, end);
    m_solvers.push_back(std::move(solver));
  }
}

bool ComparisonView::stepAll() {
  bool anyRunning = false;

  for (auto &solver : m_solvers) {
    if (solver && !solver->isFinished()) {
      solver->step();
      anyRunning = true;
    }
  }

  return anyRunning;
}

bool ComparisonView::allFinished() const {
  for (const auto &solver : m_solvers) {
    if (solver && !solver->isFinished()) {
      return false;
    }
  }
  return true;
}

void ComparisonView::resetSolvers() {
  for (auto &solver : m_solvers) {
    if (solver) {
      solver->reset();
    }
  }
  for (auto &grid : m_grids) {
    if (grid) {
      grid->clearSolverState();
    }
  }
}

ISolver *ComparisonView::getSolver(int slot) {
  if (slot >= 0 && slot < static_cast<int>(m_solvers.size())) {
    return m_solvers[slot].get();
  }
  return nullptr;
}

void ComparisonView::calculateViewports(sf::RenderWindow &window) {
  m_viewports.clear();

  sf::Vector2u size = window.getSize();
  float w = static_cast<float>(size.x);
  float h = static_cast<float>(size.y);

  // Account for UI panels
  // STATUS panel on left: ~175px
  // Stats boxes on left: ~100px
  // CONTROLS panel on right: ~260px
  float leftMargin = 175.0f;  // STATUS panel width
  float rightMargin = 260.0f; // CONTROLS panel width
  float padding = 15.0f;

  // Scale factor to shrink viewports (0.9 = 10% smaller)
  float scaleFactor = 0.9f;

  // Available space for maze viewports
  float availableWidth =
      (w - leftMargin - rightMargin - padding * 2) * scaleFactor;
  float availableHeight = (h - padding * 2) * scaleFactor;

  // Center the scaled viewports
  float extraX =
      (w - leftMargin - rightMargin - padding * 2) * (1.0f - scaleFactor) / 2;
  float extraY = (h - padding * 2) * (1.0f - scaleFactor) / 2;

  float startX = leftMargin + padding + extraX;
  float startY = padding + extraY;

  switch (m_algorithmCount) {
  case 2:
    // Side by side
    m_viewports.push_back({startX, startY, availableWidth / 2 - padding,
                           availableHeight - padding});
    m_viewports.push_back({startX + availableWidth / 2 + padding, startY,
                           availableWidth / 2 - padding,
                           availableHeight - padding});
    break;

  case 3:
    // Top row: 2, bottom row: 1 centered
    m_viewports.push_back({startX, startY, availableWidth / 2 - padding,
                           availableHeight / 2 - padding});
    m_viewports.push_back({startX + availableWidth / 2 + padding, startY,
                           availableWidth / 2 - padding,
                           availableHeight / 2 - padding});
    m_viewports.push_back(
        {startX + availableWidth / 4, startY + availableHeight / 2 + padding,
         availableWidth / 2 - padding, availableHeight / 2 - 2 * padding});
    break;

  case 4:
  default:
    // 2x2 grid
    m_viewports.push_back({startX, startY, availableWidth / 2 - padding,
                           availableHeight / 2 - padding});
    m_viewports.push_back({startX + availableWidth / 2 + padding, startY,
                           availableWidth / 2 - padding,
                           availableHeight / 2 - padding});
    m_viewports.push_back({startX, startY + availableHeight / 2 + padding,
                           availableWidth / 2 - padding,
                           availableHeight / 2 - 2 * padding});
    m_viewports.push_back({startX + availableWidth / 2 + padding,
                           startY + availableHeight / 2 + padding,
                           availableWidth / 2 - padding,
                           availableHeight / 2 - 2 * padding});
    break;
  }
}

void ComparisonView::render(sf::RenderWindow &window) {
  if (!m_visible)
    return;

  // Save current view
  sf::View currentView = window.getView();
  window.setView(window.getDefaultView());

  // Render based on menu state
  if (m_menuState == ComparisonMenuState::SelectingCount ||
      m_menuState == ComparisonMenuState::SelectingAlgorithms) {
    renderMenu(window);
  } else if (m_menuState == ComparisonMenuState::Running) {
    // Calculate viewports (in the maze area, leaving space for UI panels)
    calculateViewports(window);

    // Render each viewport
    for (int i = 0;
         i < m_algorithmCount && i < static_cast<int>(m_viewports.size());
         ++i) {
      renderViewport(window, i, m_viewports[i]);
    }

    // Render RESULTS panel on left side (always visible, not just when
    // finished)
    renderStatsPanel(window);
  }

  // Restore view
  window.setView(currentView);
}

void ComparisonView::renderMenu(sf::RenderWindow &window) {
  sf::Vector2u size = window.getSize();
  float centerX = size.x / 2.0f;
  float centerY = size.y / 2.0f;

  float menuWidth = 400.0f;
  float menuHeight = 300.0f;
  float menuX = centerX - menuWidth / 2;
  float menuY = centerY - menuHeight / 2;

  // Background panel
  sf::RectangleShape bg({menuWidth, menuHeight});
  bg.setPosition({menuX, menuY});
  bg.setFillColor(sf::Color{30, 30, 40, 240});
  bg.setOutlineColor(sf::Color{0, 229, 255});
  bg.setOutlineThickness(2.0f);
  window.draw(bg);

  // Title
  renderText(window, "KARSILASTIRMA MODU", menuX + 20, menuY + 15,
             sf::Color{0, 229, 255}, 20);

  if (m_menuState == ComparisonMenuState::SelectingCount) {
    // Algorithm count selection
    renderText(window, "Algoritma Sayisi Secin:", menuX + 20, menuY + 60,
               sf::Color::White, 16);

    renderText(window, "[2] - 2 Algoritma", menuX + 50, menuY + 100,
               sf::Color{255, 214, 0}, 18);
    renderText(window, "[4] - 4 Algoritma", menuX + 50, menuY + 140,
               sf::Color{255, 214, 0}, 18);

    renderText(window, "ESC - Iptal", menuX + 20, menuY + 250,
               sf::Color{128, 128, 128}, 14);

  } else if (m_menuState == ComparisonMenuState::SelectingAlgorithms) {
    // Algorithm selection
    std::string countStr =
        std::to_string(m_algorithmCount) + " Algoritma Secili";
    renderText(window, countStr, menuX + 20, menuY + 50, sf::Color::White, 14);

    renderText(window, "Yukari/Asagi: Slot sec | Sol/Sag: Algoritma degistir",
               menuX + 20, menuY + 75, sf::Color{128, 128, 128}, 12);

    float y = menuY + 110;
    for (int i = 0; i < m_algorithmCount; ++i) {
      sf::Color color =
          (i == m_menuSelection) ? sf::Color{0, 255, 127} : sf::Color::White;
      std::string prefix = (i == m_menuSelection) ? "> " : "  ";
      std::string line = prefix + "Slot " + std::to_string(i + 1) + ": < " +
                         getAlgorithmName(m_selectedAlgorithms[i]) + " >";
      renderText(window, line, menuX + 30, y, color, 16);
      y += 30;
    }

    renderText(window, "ENTER - Baslat | ESC - Geri", menuX + 20, menuY + 250,
               sf::Color{128, 128, 128}, 14);
  }
}

void ComparisonView::renderViewport(sf::RenderWindow &window, int slot,
                                    const ViewportRect &vp) {
  // Draw viewport background
  sf::RectangleShape bg({vp.width, vp.height});
  bg.setPosition({vp.x, vp.y});
  bg.setFillColor(sf::Color{30, 30, 30});
  bg.setOutlineColor(s_slotColors[slot % 4]);
  bg.setOutlineThickness(2.0f);
  window.draw(bg);

  // Draw algorithm name
  int algoIdx = m_selectedAlgorithms[slot];
  std::string name = getAlgorithmName(algoIdx);
  renderText(window, name, vp.x + 10, vp.y + 5, s_slotColors[slot % 4], 16);

  // Draw stats if solver exists
  if (slot < static_cast<int>(m_solvers.size()) && m_solvers[slot]) {
    auto stats = m_solvers[slot]->getStats();
    std::ostringstream ss;
    ss << "V:" << stats.nodesVisited << " P:" << stats.pathLength
       << " T:" << std::fixed << std::setprecision(1) << stats.elapsedTime
       << "ms";
    renderText(window, ss.str(), vp.x + vp.width - 200, vp.y + 5,
               sf::Color::White, 12);
  }

  // Render mini grid
  if (slot < static_cast<int>(m_grids.size()) && m_grids[slot]) {
    ViewportRect gridVp = {vp.x + 5, vp.y + 30, vp.width - 10, vp.height - 35};
    renderMiniGrid(window, *m_grids[slot], gridVp, s_slotColors[slot % 4]);
  }
}

void ComparisonView::renderMiniGrid(sf::RenderWindow &window, Grid &grid,
                                    const ViewportRect &vp,
                                    const sf::Color &visitedColor) {
  int w = grid.getWidth();
  int h = grid.getHeight();

  float cellW = vp.width / w;
  float cellH = vp.height / h;
  float cellSize = std::min(cellW, cellH);

  // Center the grid
  float offsetX = vp.x + (vp.width - cellSize * w) / 2;
  float offsetY = vp.y + (vp.height - cellSize * h) / 2;

  float wallThickness = std::max(1.0f, cellSize * 0.1f);
  sf::RectangleShape cell({cellSize - 1, cellSize - 1});
  sf::RectangleShape wallH({cellSize, wallThickness});
  sf::RectangleShape wallV({wallThickness, cellSize});

  for (int y = 0; y < h; ++y) {
    for (int x = 0; x < w; ++x) {
      float px = offsetX + x * cellSize;
      float py = offsetY + y * cellSize;
      cell.setPosition({px, py});

      uint8_t flags = grid.getCell(x, y);

      // Determine color based on terrain first
      TerrainType terrain = grid.getTerrain(x, y);
      sf::Color color = getTerrainColor(terrain);

      // Override with solver state colors - InPath has highest priority
      if (hasFlag(flags, CellFlags::InPath)) {
        color = sf::Color{255, 0, 255}; // Magenta for path (full cell)
      } else if (hasFlag(flags, CellFlags::Current)) {
        color = sf::Color{255, 255, 0}; // Yellow for current
      } else if (hasFlag(flags, CellFlags::Visited)) {
        color = visitedColor;
      }

      // Start/end markers
      Position pos{x, y};
      if (pos == grid.getStart()) {
        color = sf::Color{0, 255, 127}; // Green
      } else if (pos == grid.getEnd()) {
        color = sf::Color{255, 69, 0}; // Red
      }

      cell.setFillColor(color);
      window.draw(cell);

      // Draw walls
      sf::Color wallColor{200, 200, 200};

      // North wall
      if (hasFlag(flags, CellFlags::WallNorth)) {
        wallH.setFillColor(wallColor);
        wallH.setPosition({px, py});
        window.draw(wallH);
      }
      // West wall
      if (hasFlag(flags, CellFlags::WallWest)) {
        wallV.setFillColor(wallColor);
        wallV.setPosition({px, py});
        window.draw(wallV);
      }
      // South wall (only on bottom edge)
      if (y == h - 1 && hasFlag(flags, CellFlags::WallSouth)) {
        wallH.setFillColor(wallColor);
        wallH.setPosition({px, py + cellSize - wallThickness});
        window.draw(wallH);
      }
      // East wall (only on right edge)
      if (x == w - 1 && hasFlag(flags, CellFlags::WallEast)) {
        wallV.setFillColor(wallColor);
        wallV.setPosition({px + cellSize - wallThickness, py});
        window.draw(wallV);
      }

      // Path is rendered via cell color, not overlay
    }
  }
}

void ComparisonView::renderResults(sf::RenderWindow &window) {
  if (!m_fontLoaded)
    return;

  sf::Vector2u size = window.getSize();
  float tableWidth = 400.0f;
  float tableHeight = 30.0f + m_algorithmCount * 25.0f;
  float x = (size.x - tableWidth) / 2;
  float y = size.y - tableHeight - 20;

  // Background
  sf::RectangleShape bg({tableWidth, tableHeight});
  bg.setPosition({x, y});
  bg.setFillColor(sf::Color{20, 20, 20, 230});
  bg.setOutlineColor(sf::Color{60, 60, 60});
  bg.setOutlineThickness(1.0f);
  window.draw(bg);

  // Header
  renderText(window, "RESULTS", x + 10, y + 5, sf::Color{255, 214, 0}, 14);
  renderText(window, "Visited", x + 120, y + 5, sf::Color::White, 12);
  renderText(window, "Path", x + 200, y + 5, sf::Color::White, 12);
  renderText(window, "Time", x + 280, y + 5, sf::Color::White, 12);

  // Results for each solver
  float rowY = y + 28;
  size_t bestPath = SIZE_MAX;
  size_t bestVisited = SIZE_MAX;

  // Find best values
  for (const auto &solver : m_solvers) {
    if (solver && solver->foundPath()) {
      auto stats = solver->getStats();
      bestPath = std::min(bestPath, stats.pathLength);
      bestVisited = std::min(bestVisited, stats.nodesVisited);
    }
  }

  for (int i = 0; i < m_algorithmCount; ++i) {
    if (i >= static_cast<int>(m_solvers.size()))
      break;

    auto *solver = m_solvers[i].get();
    if (!solver)
      continue;

    auto stats = solver->getStats();
    std::string name = getAlgorithmName(m_selectedAlgorithms[i]);

    renderText(window, name, x + 10, rowY, s_slotColors[i % 4], 12);

    // Highlight best values
    sf::Color visitedColor = (stats.nodesVisited == bestVisited)
                                 ? sf::Color{0, 255, 127}
                                 : sf::Color::White;
    sf::Color pathColor = (stats.pathLength == bestPath && solver->foundPath())
                              ? sf::Color{0, 255, 127}
                              : sf::Color::White;

    renderText(window, std::to_string(stats.nodesVisited), x + 120, rowY,
               visitedColor, 12);

    std::string pathStr =
        solver->foundPath() ? std::to_string(stats.pathLength) : "-";
    renderText(window, pathStr, x + 200, rowY, pathColor, 12);

    std::ostringstream ss;
    ss << std::fixed << std::setprecision(1) << stats.elapsedTime << "ms";
    renderText(window, ss.str(), x + 280, rowY, sf::Color::White, 12);

    rowY += 22;
  }
}

void ComparisonView::renderText(sf::RenderWindow &window,
                                const std::string &text, float x, float y,
                                const sf::Color &color, unsigned int size) {
  if (!m_fontLoaded)
    return;

  sf::Text sfText(*m_font, text, size);
  sfText.setPosition({x, y});
  sfText.setFillColor(color);
  window.draw(sfText);
}

void ComparisonView::renderStatsPanel(sf::RenderWindow &window) {
  if (!m_fontLoaded)
    return;

  float panelWidth = 220.0f;
  float x = 20.0f;  // Padding from left
  float y = 230.0f; // Start below main status panel

  // Title
  renderText(window, "RESULTS", x, y, sf::Color{255, 214, 0}, 16);
  y += 25.0f;

  for (int i = 0; i < m_algorithmCount; ++i) {
    if (i >= static_cast<int>(m_solvers.size()))
      break;

    auto *solver = m_solvers[i].get();
    if (!solver)
      continue;

    // Skip if not running or done? No, show always.

    auto stats = solver->getStats();
    sf::Color color = s_slotColors[i % 4];

    // Card Background
    float cardHeight = 135.0f; // Increased for extra details
    sf::RectangleShape card({panelWidth, cardHeight});
    card.setPosition({x, y});
    card.setFillColor(sf::Color{30, 30, 30, 240});
    card.setOutlineColor(color);
    card.setOutlineThickness(1.0f);
    window.draw(card);

    // Content
    float tx = x + 10.0f;
    float ty = y + 8.0f;
    float lh = 20.0f;

    // Name
    std::string name = getAlgorithmName(m_selectedAlgorithms[i]);
    // Abbreviate common long names?
    if (name == "Breadth-First Search")
      name = "BFS";
    else if (name == "Depth-First Search")
      name = "DFS";
    else if (name == "Bidirectional BFS")
      name = "Bi-BFS";
    else if (name == "Greedy Best-First")
      name = "Greedy";

    renderText(window, name, tx, ty, color, 15);
    ty += 24.0f;

    // Stats Rows
    auto drawStat = [&](const std::string &label, const std::string &val,
                        const sf::Color &valColor) {
      renderText(window, label, tx, ty, sf::Color::White, 13);
      renderText(window, val, tx + 70.0f, ty, valColor, 13);
      ty += lh;
    };

    std::stringstream ss;
    ss << stats.nodesVisited;
    drawStat("Visited:", ss.str(), sf::Color{0, 255, 255}); // Cyan

    ss.str("");
    ss << stats.nodesInQueue;
    drawStat("Queue:", ss.str(), sf::Color{0, 255, 255});

    ss.str("");
    ss << (solver->foundPath() ? std::to_string(stats.pathLength) : "-");
    drawStat("Path:", ss.str(), sf::Color{0, 255, 255});

    ss.str("");
    ss << std::fixed << std::setprecision(1) << stats.elapsedTime << " ms";
    drawStat("Time:", ss.str(), sf::Color{0, 255, 255});

    y += cardHeight + 12.0f;
  }
}

} // namespace maze
