#include "ComparisonView.hpp"
#include "../Algorithms/AStarSolver.hpp"
#include "../Algorithms/BFSSolver.hpp"
#include "../Algorithms/BidirectionalBFS.hpp"
#include "../Algorithms/DFSSolver.hpp"
#include "../Algorithms/DijkstraSolver.hpp"
#include "../Algorithms/GreedySolver.hpp"
#include "Visualization/Camera.hpp"
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

void ComparisonView::zoom(float factor) {
  m_zoom *= factor;
  if (m_zoom > 10.0f)
    m_zoom = 10.0f;
  if (m_zoom < 0.1f)
    m_zoom = 0.1f;
}

void ComparisonView::fitToMaze() {
  if (m_viewports.empty() || !m_grid)
    return;

  // We use the first viewport to calculate the ideal fit.
  auto &vp = m_viewports[0];

  // Available space inside the sub-panel (deducting headers/padding)
  float availableW = vp.width - 10.0f;
  float availableH = vp.height - 35.0f;

  if (availableW <= 0 || availableH <= 0)
    return;

  // Maze virtual bounds
  int w = m_grid->getWidth();
  int h = m_grid->getHeight();
  float virtualMazeW = (w * 2 + 1) * 10.0f;
  float virtualMazeH = (h * 2 + 1) * 10.0f;

  float paddingFactor = 0.9f;
  float zoomX = (availableW * paddingFactor) / virtualMazeW;
  float zoomY = (availableH * paddingFactor) / virtualMazeH;

  m_zoom = std::min(zoomX, zoomY);
  m_pan = {0.0f, 0.0f}; // Center offset resets to 0
}

void ComparisonView::pan(float dx, float dy) {
  float speed =
      1.0f / m_zoom; // Align with exact mouse movement mapped to scale
  m_pan.x -= dx * speed;
  m_pan.y -= dy * speed;
}

void ComparisonView::calculateViewports(sf::RenderWindow &window) {
  m_viewports.clear();

  sf::Vector2u size = window.getSize();
  float w = static_cast<float>(size.x);
  float h = static_cast<float>(size.y);

  // Account for UI panels
  // STATUS & RESULTS panel on left: ~200px
  // CONTROLS panel on right: ~260px
  float leftMargin = 210.0f;  // STATUS/RESULTS panel width
  float rightMargin = 260.0f; // CONTROLS panel width
  float padding = 15.0f;

  // Scale factor to shrink viewports slightly so they don't hit edges
  float scaleFactor = 0.85f;

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

void ComparisonView::render(sf::RenderWindow &window, class Camera *camera) {
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
      renderViewport(window, i, m_viewports[i], camera);
    }

    // Restore view for UI overlay
    window.setView(window.getDefaultView());

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
                                    const ViewportRect &vp,
                                    class Camera *camera) {
  // CRITICAL FIX: Always start by drawing UI elements in the default window
  // coordinate space before we potentially switch to a camera viewport. This
  // prevents 'Z-offset overlap' where the UI draws locally into the previous
  // algorithm's clipped viewport.
  window.setView(window.getDefaultView());

  // Draw viewport background and border
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

  // Render mini grid with its own camera-controlled view
  if (slot < static_cast<int>(m_grids.size()) && m_grids[slot]) {
    ViewportRect gridVp = {vp.x + 5, vp.y + 30, vp.width - 10, vp.height - 35};
    renderMiniGrid(window, *m_grids[slot], gridVp, s_slotColors[slot % 4],
                   camera);
  }
}

void ComparisonView::renderMiniGrid(sf::RenderWindow &window, Grid &grid,
                                    const ViewportRect &vp,
                                    const sf::Color &visitedColor,
                                    class Camera *camera) {
  int w = grid.getWidth();
  int h = grid.getHeight();

  // Draw as a (2*w+1) x (2*h+1) tilemap
  int tilesX = w * 2 + 1;
  int tilesY = h * 2 + 1;

  // The base physical size of a tile in our virtual world
  float tileSize = 10.0f;

  // Total virtual size of the maze
  float virtualMazeW = tilesX * tileSize;
  float virtualMazeH = tilesY * tileSize;

  // Set SFML Viewport to clip to this grid's rectangle
  sf::Vector2u winSize = window.getSize();
  sf::View view;

  sf::Vector2f viewSize;
  viewSize.x = vp.width / m_zoom;
  viewSize.y = vp.height / m_zoom;

  view.setSize(viewSize);

  // Center is the middle of the maze adjusted by user panning.
  sf::Vector2f center((virtualMazeW / 2.0f) + m_pan.x,
                      (virtualMazeH / 2.0f) + m_pan.y);
  view.setCenter(center);

  // Set physical viewport rendering bounds
  view.setViewport(
      sf::FloatRect({vp.x / winSize.x, vp.y / winSize.y},
                    {vp.width / winSize.x, vp.height / winSize.y}));
  window.setView(view);

  sf::RectangleShape tileShape({tileSize, tileSize});
  sf::Color wallColor = sf::Color::Black; // Walls are black squares

  // Helper lambda to get cell color
  auto getCellVisualColor = [&](int cx, int cy) -> sf::Color {
    uint8_t flags = grid.getCell(cx, cy);
    TerrainType terrain = grid.getTerrain(cx, cy);
    sf::Color color = getTerrainColor(terrain);

    if (hasFlag(flags, CellFlags::InPath)) {
      color = sf::Color{255, 0, 255}; // Path magenta
    } else if (hasFlag(flags, CellFlags::Current)) {
      color = sf::Color{255, 255, 0}; // Yellow
    } else if (hasFlag(flags, CellFlags::Visited)) {
      color = visitedColor;
    }

    Position pos{cx, cy};
    if (pos == grid.getStart()) {
      color = sf::Color{0, 255, 127};
    } else if (pos == grid.getEnd()) {
      color = sf::Color{255, 69, 0};
    }
    return color;
  };

  // Helper lambda to get passage color between c1 and c2
  auto getPassageColor = [&](int c1x, int c1y, int c2x, int c2y) -> sf::Color {
    uint8_t f1 = grid.getCell(c1x, c1y);
    uint8_t f2 = grid.getCell(c2x, c2y);

    if (hasFlag(f1, CellFlags::InPath) && hasFlag(f2, CellFlags::InPath))
      return sf::Color{255, 0, 255};
    if (hasFlag(f1, CellFlags::Visited) && hasFlag(f2, CellFlags::Visited))
      return visitedColor;

    TerrainType t1 = grid.getTerrain(c1x, c1y);
    TerrainType t2 = grid.getTerrain(c2x, c2y);
    TerrainType terrain = (t1 != TerrainType::Normal) ? t1 : t2;
    return getTerrainColor(terrain);
  };

  for (int ty = 0; ty < tilesY; ++ty) {
    for (int tx = 0; tx < tilesX; ++tx) {
      float px = tx * tileSize;
      float py = ty * tileSize;
      tileShape.setPosition({px, py});

      bool isWallX = (tx % 2 == 0);
      bool isWallY = (ty % 2 == 0);

      // Four cases:
      if (isWallX && isWallY) {
        // Corner - always wall
        tileShape.setFillColor(wallColor);
      } else if (!isWallX && !isWallY) {
        // Cell
        int cx = (tx - 1) / 2;
        int cy = (ty - 1) / 2;
        tileShape.setFillColor(getCellVisualColor(cx, cy));
      } else if (isWallX && !isWallY) {
        // Vertical wall or passage between (cx-1, cy) and (cx, cy)
        int cx = tx / 2;
        int cy = (ty - 1) / 2;
        if (cx == 0 || cx == w) {
          tileShape.setFillColor(wallColor); // Outer boundary
        } else {
          // Check wall flag for cell to the right (cx)
          if (hasFlag(grid.getCell(cx, cy), CellFlags::WallWest)) {
            tileShape.setFillColor(wallColor);
          } else {
            tileShape.setFillColor(getPassageColor(cx - 1, cy, cx, cy));
          }
        }
      } else if (!isWallX && isWallY) {
        // Horizontal wall or passage between (cx, cy-1) and (cx, cy)
        int cx = (tx - 1) / 2;
        int cy = ty / 2;
        if (cy == 0 || cy == h) {
          tileShape.setFillColor(wallColor); // Outer boundary
        } else {
          // Check wall flag for cell below (cy)
          if (hasFlag(grid.getCell(cx, cy), CellFlags::WallNorth)) {
            tileShape.setFillColor(wallColor);
          } else {
            tileShape.setFillColor(getPassageColor(cx, cy - 1, cx, cy));
          }
        }
      }

      // Draw the tile slightly expanded or exactly?
      // Since float coordinates might cause small gaps, we can add a slight
      // overlap: tileShape.setSize({tileSize + 0.5f, tileSize + 0.5f});
      window.draw(tileShape);
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
