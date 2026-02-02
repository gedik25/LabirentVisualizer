#include "ComparisonView.hpp"
#include "../Algorithms/BFSSolver.hpp"
#include "../Algorithms/DFSSolver.hpp"
#include "../Algorithms/AStarSolver.hpp"
#include "../Algorithms/DijkstraSolver.hpp"
#include "../Algorithms/GreedySolver.hpp"
#include "../Algorithms/BidirectionalBFS.hpp"
#include <sstream>
#include <iomanip>

namespace maze {

const std::array<sf::Color, 4> ComparisonView::s_slotColors = {{
    sf::Color{0, 229, 255},    // Cyan (BFS default)
    sf::Color{255, 214, 0},    // Yellow (DFS default)
    sf::Color{255, 0, 255},    // Magenta
    sf::Color{0, 255, 127}     // Spring Green
}};

ComparisonView::ComparisonView() {
    m_selectedAlgorithms = {0, 1, 2, 3};  // Default: BFS, DFS, A*, Dijkstra
}

bool ComparisonView::init(sf::RenderWindow& window) {
    // Load font
    std::vector<std::string> fontPaths = {
        "/System/Library/Fonts/SFNSMono.ttf",
        "/System/Library/Fonts/Menlo.ttc",
        "/System/Library/Fonts/Monaco.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf",
        "C:/Windows/Fonts/consola.ttf"
    };
    
    m_font.emplace();
    for (const auto& path : fontPaths) {
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

void ComparisonView::setGrid(std::shared_ptr<Grid> grid) {
    m_grid = grid;
}

std::string ComparisonView::getAlgorithmName(int index) {
    switch (index) {
        case 0: return "BFS";
        case 1: return "DFS";
        case 2: return "A*";
        case 3: return "Dijkstra";
        case 4: return "Greedy";
        case 5: return "BiDir BFS";
        default: return "Unknown";
    }
}

std::unique_ptr<ISolver> ComparisonView::createSolver(int index, std::shared_ptr<Grid> grid) {
    switch (index) {
        case 0: return std::make_unique<BFSSolver>(grid);
        case 1: return std::make_unique<DFSSolver>(grid);
        case 2: return std::make_unique<AStarSolver>(grid);
        case 3: return std::make_unique<DijkstraSolver>(grid);
        case 4: return std::make_unique<GreedySolver>(grid);
        case 5: return std::make_unique<BidirectionalBFS>(grid);
        default: return std::make_unique<BFSSolver>(grid);
    }
}

void ComparisonView::initSolvers(const Position& start, const Position& end) {
    m_solvers.clear();
    m_grids.clear();
    
    if (!m_grid) return;
    
    // Create copies of the grid for each solver
    for (int i = 0; i < m_algorithmCount; ++i) {
        // Create a copy of the grid
        auto gridCopy = std::make_shared<Grid>(m_grid->getWidth(), m_grid->getHeight());
        
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
    
    for (auto& solver : m_solvers) {
        if (solver && !solver->isFinished()) {
            solver->step();
            anyRunning = true;
        }
    }
    
    return anyRunning;
}

bool ComparisonView::allFinished() const {
    for (const auto& solver : m_solvers) {
        if (solver && !solver->isFinished()) {
            return false;
        }
    }
    return true;
}

void ComparisonView::resetSolvers() {
    for (auto& solver : m_solvers) {
        if (solver) {
            solver->reset();
        }
    }
    for (auto& grid : m_grids) {
        if (grid) {
            grid->clearSolverState();
        }
    }
}

ISolver* ComparisonView::getSolver(int slot) {
    if (slot >= 0 && slot < static_cast<int>(m_solvers.size())) {
        return m_solvers[slot].get();
    }
    return nullptr;
}

void ComparisonView::calculateViewports(sf::RenderWindow& window) {
    m_viewports.clear();
    
    sf::Vector2u size = window.getSize();
    float w = static_cast<float>(size.x);
    float h = static_cast<float>(size.y);
    float padding = 10.0f;
    float headerHeight = 40.0f;
    
    switch (m_algorithmCount) {
        case 2:
            // Side by side
            m_viewports.push_back({padding, headerHeight, w/2 - 2*padding, h - headerHeight - padding});
            m_viewports.push_back({w/2 + padding, headerHeight, w/2 - 2*padding, h - headerHeight - padding});
            break;
            
        case 3:
            // Top row: 2, bottom row: 1 centered
            m_viewports.push_back({padding, headerHeight, w/2 - 2*padding, h/2 - headerHeight});
            m_viewports.push_back({w/2 + padding, headerHeight, w/2 - 2*padding, h/2 - headerHeight});
            m_viewports.push_back({w/4 + padding, h/2 + padding, w/2 - 2*padding, h/2 - 2*padding});
            break;
            
        case 4:
        default:
            // 2x2 grid
            m_viewports.push_back({padding, headerHeight, w/2 - 2*padding, h/2 - headerHeight});
            m_viewports.push_back({w/2 + padding, headerHeight, w/2 - 2*padding, h/2 - headerHeight});
            m_viewports.push_back({padding, h/2 + padding, w/2 - 2*padding, h/2 - 2*padding});
            m_viewports.push_back({w/2 + padding, h/2 + padding, w/2 - 2*padding, h/2 - 2*padding});
            break;
    }
}

void ComparisonView::render(sf::RenderWindow& window) {
    if (!m_visible) return;
    
    // Save current view
    sf::View currentView = window.getView();
    window.setView(window.getDefaultView());
    
    // Clear with dark background
    window.clear(sf::Color{18, 18, 18});
    
    // Calculate viewports
    calculateViewports(window);
    
    // Render each viewport
    for (int i = 0; i < m_algorithmCount && i < static_cast<int>(m_viewports.size()); ++i) {
        renderViewport(window, i, m_viewports[i]);
    }
    
    // Render results if all finished
    if (allFinished()) {
        renderResults(window);
    }
    
    // Restore view
    window.setView(currentView);
}

void ComparisonView::renderViewport(sf::RenderWindow& window, int slot, const ViewportRect& vp) {
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
           << " T:" << std::fixed << std::setprecision(1) << stats.elapsedTime << "ms";
        renderText(window, ss.str(), vp.x + vp.width - 200, vp.y + 5, sf::Color::White, 12);
    }
    
    // Render mini grid
    if (slot < static_cast<int>(m_grids.size()) && m_grids[slot]) {
        ViewportRect gridVp = {vp.x + 5, vp.y + 30, vp.width - 10, vp.height - 35};
        renderMiniGrid(window, *m_grids[slot], gridVp, s_slotColors[slot % 4]);
    }
}

void ComparisonView::renderMiniGrid(sf::RenderWindow& window, Grid& grid, 
                                     const ViewportRect& vp, const sf::Color& visitedColor) {
    int w = grid.getWidth();
    int h = grid.getHeight();
    
    float cellW = vp.width / w;
    float cellH = vp.height / h;
    float cellSize = std::min(cellW, cellH);
    
    // Center the grid
    float offsetX = vp.x + (vp.width - cellSize * w) / 2;
    float offsetY = vp.y + (vp.height - cellSize * h) / 2;
    
    sf::RectangleShape cell({cellSize - 1, cellSize - 1});
    
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            float px = offsetX + x * cellSize;
            float py = offsetY + y * cellSize;
            cell.setPosition({px, py});
            
            uint8_t flags = grid.getCell(x, y);
            
            // Determine color
            sf::Color color{40, 40, 40};  // Default
            
            if (hasFlag(flags, CellFlags::InPath)) {
                color = sf::Color{255, 0, 255};  // Magenta for path
            } else if (hasFlag(flags, CellFlags::Visited)) {
                color = visitedColor;
            }
            
            // Start/end markers
            Position pos{x, y};
            if (pos == grid.getStart()) {
                color = sf::Color{0, 255, 127};  // Green
            } else if (pos == grid.getEnd()) {
                color = sf::Color{255, 69, 0};   // Red
            }
            
            cell.setFillColor(color);
            window.draw(cell);
        }
    }
}

void ComparisonView::renderResults(sf::RenderWindow& window) {
    if (!m_fontLoaded) return;
    
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
    for (const auto& solver : m_solvers) {
        if (solver && solver->foundPath()) {
            auto stats = solver->getStats();
            bestPath = std::min(bestPath, stats.pathLength);
            bestVisited = std::min(bestVisited, stats.nodesVisited);
        }
    }
    
    for (int i = 0; i < m_algorithmCount; ++i) {
        if (i >= static_cast<int>(m_solvers.size())) break;
        
        auto* solver = m_solvers[i].get();
        if (!solver) continue;
        
        auto stats = solver->getStats();
        std::string name = getAlgorithmName(m_selectedAlgorithms[i]);
        
        renderText(window, name, x + 10, rowY, s_slotColors[i % 4], 12);
        
        // Highlight best values
        sf::Color visitedColor = (stats.nodesVisited == bestVisited) ? 
            sf::Color{0, 255, 127} : sf::Color::White;
        sf::Color pathColor = (stats.pathLength == bestPath && solver->foundPath()) ? 
            sf::Color{0, 255, 127} : sf::Color::White;
        
        renderText(window, std::to_string(stats.nodesVisited), x + 120, rowY, visitedColor, 12);
        
        std::string pathStr = solver->foundPath() ? std::to_string(stats.pathLength) : "-";
        renderText(window, pathStr, x + 200, rowY, pathColor, 12);
        
        std::ostringstream ss;
        ss << std::fixed << std::setprecision(1) << stats.elapsedTime << "ms";
        renderText(window, ss.str(), x + 280, rowY, sf::Color::White, 12);
        
        rowY += 22;
    }
}

void ComparisonView::renderText(sf::RenderWindow& window, const std::string& text,
                                 float x, float y, const sf::Color& color, unsigned int size) {
    if (!m_fontLoaded) return;
    
    sf::Text sfText(*m_font, text, size);
    sfText.setPosition({x, y});
    sfText.setFillColor(color);
    window.draw(sfText);
}

} // namespace maze
