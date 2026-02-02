#include <SFML/Graphics.hpp>
#include <chrono>
#include <iostream>
#include <memory>
#include <thread>

// Core
#include "Core/DifficultySettings.hpp"
#include "Core/Grid.hpp"
#include "Core/Maze.hpp"
#include "Core/SeedManager.hpp"
#include "Core/Terrain.hpp"

// Algorithms - Solvers
#include "Algorithms/AStarSolver.hpp"
#include "Algorithms/BFSSolver.hpp"
#include "Algorithms/BidirectionalBFS.hpp"
#include "Algorithms/DFSSolver.hpp"
#include "Algorithms/DijkstraSolver.hpp"
#include "Algorithms/GreedySolver.hpp"

// Generators
#include "Generators/BinaryTreeGenerator.hpp"
#include "Generators/EllersGenerator.hpp"
#include "Generators/IMazeGenerator.hpp"
#include "Generators/KruskalGenerator.hpp"
#include "Generators/PrimGenerator.hpp"

// Visualization
#include "Visualization/Camera.hpp"
#include "Visualization/Renderer.hpp"
#include "Visualization/Theme.hpp"
#include "Visualization/UIPanel.hpp"

// UI
#include "UI/ComparisonView.hpp"
#include "UI/MenuSystem.hpp"

using namespace maze;

// Use AppState from maze namespace (defined in UIPanel.hpp)
using AppState = maze::AppState;

// Generator type enum
enum class GeneratorType {
  RecursiveBacktracking = 0,
  Prim,
  Kruskal,
  BinaryTree,
  Ellers
};

// Solver type enum
enum class SolverType { BFS = 0, DFS, AStar, Dijkstra, Greedy, Bidirectional };

// Configuration
struct Config {
  int gridWidth = 25;
  int gridHeight = 25;
  int animationDelay = 10; // milliseconds between steps
  bool animationEnabled = true;
  int difficulty = 3; // 1-5
  GeneratorType generatorType = GeneratorType::RecursiveBacktracking;
  SolverType solverType = SolverType::BFS;
  bool terrainEnabled = false;

  // Preset sizes
  static constexpr int PresetSmall = 25;
  static constexpr int PresetMedium = 100;
  static constexpr int PresetLarge = 500;

  // Animation thresholds
  static constexpr int AnimationDisableThreshold = 500;
  static constexpr int MinDelay = 1;
  static constexpr int MaxDelay = 200;
};

void printHelp() {
  std::cout << "\n=== Maze Visualizer Controls ===\n"
            << "M        - Open/Close Menu\n"
            << "G        - Generate new maze (animated)\n"
            << "Enter    - Generate new maze (instant)\n"
            << "Space    - Toggle animation ON/OFF\n"
            << "T        - Toggle terrain ON/OFF\n"
            << "\n--- Pathfinding Algorithms ---\n"
            << "B        - BFS (Breadth-First Search)\n"
            << "D        - DFS (Depth-First Search)\n"
            << "A        - A* (A-Star)\n"
            << "J        - Dijkstra\n"
            << "Y        - Greedy Best-First\n"
            << "I        - Bidirectional BFS\n"
            << "\n--- Maze Generators ---\n"
            << "Shift+1  - Recursive Backtracking\n"
            << "Shift+2  - Prim's Algorithm\n"
            << "Shift+3  - Kruskal's Algorithm\n"
            << "Shift+4  - Binary Tree\n"
            << "Shift+5  - Eller's Algorithm\n"
            << "\n--- Other ---\n"
            << "R        - Reset (clear solution)\n"
            << "C        - Clear all\n"
            << "+/-      - Adjust animation speed\n"
            << "WASD     - Pan camera\n"
            << "Scroll   - Zoom in/out\n"
            << "F        - Fit maze to window\n"
            << "1/2/3    - Size presets\n"
            << "H        - Toggle help panel\n"
            << "ESC      - Exit\n"
            << "================================\n\n";
}

void printStatus(AppState state, const Config &config,
                 ISolver *solver = nullptr) {
  std::cout << "\r";
  std::cout << "[" << config.gridWidth << "x" << config.gridHeight << "] ";
  std::cout << "Anim: " << (config.animationEnabled ? "ON " : "OFF") << " | ";
  std::cout << "Delay: " << config.animationDelay << "ms | ";
  std::cout << "Terrain: " << (config.terrainEnabled ? "ON " : "OFF") << " | ";

  switch (state) {
  case AppState::Idle:
    std::cout << "Ready - Press G to generate maze        ";
    break;
  case AppState::Generating:
    std::cout << "Generating maze...                      ";
    break;
  case AppState::Solving:
    if (solver) {
      auto stats = solver->getStats();
      std::cout << solver->getName() << " | Visited: " << stats.nodesVisited;
      std::cout << " | Queue: " << stats.nodesInQueue << "        ";
    }
    break;
  case AppState::Finished:
    if (solver) {
      auto stats = solver->getStats();
      if (solver->foundPath()) {
        std::cout << "Path found! Length: " << stats.pathLength;
        std::cout << " | Visited: " << stats.nodesVisited;
        std::cout << " | Time: " << stats.elapsedTime << "ms   ";
      } else {
        std::cout << "No path exists!                         ";
      }
    }
    break;
  case AppState::Comparing:
    std::cout << "Comparison mode active                  ";
    break;
  }
  std::cout << std::flush;
}

std::string getGeneratorName(GeneratorType type) {
  switch (type) {
  case GeneratorType::RecursiveBacktracking:
    return "Recursive Backtracking";
  case GeneratorType::Prim:
    return "Prim's Algorithm";
  case GeneratorType::Kruskal:
    return "Kruskal's Algorithm";
  case GeneratorType::BinaryTree:
    return "Binary Tree";
  case GeneratorType::Ellers:
    return "Eller's Algorithm";
  }
  return "Unknown";
}

std::string getSolverName(SolverType type) {
  switch (type) {
  case SolverType::BFS:
    return "BFS";
  case SolverType::DFS:
    return "DFS";
  case SolverType::AStar:
    return "A*";
  case SolverType::Dijkstra:
    return "Dijkstra";
  case SolverType::Greedy:
    return "Greedy";
  case SolverType::Bidirectional:
    return "Bidirectional BFS";
  }
  return "Unknown";
}

int main() {
  printHelp();

  Config config;
  SeedManager seedManager;

  // Create window
  const unsigned int windowWidth = 1600;
  const unsigned int windowHeight = 1000;

  sf::RenderWindow window(sf::VideoMode({windowWidth, windowHeight}),
                          "Maze Visualizer - Labirent", sf::Style::Default);
  window.setFramerateLimit(60);

  // Initialize components
  auto grid = std::make_shared<Grid>(config.gridWidth, config.gridHeight);
  auto legacyMazeGen = std::make_unique<Maze>(grid);

  // New generators
  std::unique_ptr<IMazeGenerator> currentGenerator;
  auto primGen = std::make_unique<PrimGenerator>();
  auto kruskalGen = std::make_unique<KruskalGenerator>();
  auto binaryGen = std::make_unique<BinaryTreeGenerator>();
  auto ellersGen = std::make_unique<EllersGenerator>();

  Camera camera(static_cast<float>(windowWidth),
                static_cast<float>(windowHeight));
  Renderer renderer(window, camera);
  renderer.setGrid(grid);
  renderer.autoCalculateCellSize();
  camera.fitToMaze(grid->getWidth(), grid->getHeight(), renderer.getCellSize());

  // UI Panel
  UIPanel uiPanel;
  if (!uiPanel.init(window)) {
    std::cout << "Warning: Could not load font, UI panel disabled\n";
  }
  uiPanel.setGridSize(config.gridWidth, config.gridHeight);
  uiPanel.setAnimationEnabled(config.animationEnabled);
  uiPanel.setAnimationDelay(config.animationDelay);
  uiPanel.setGeneratorName(getGeneratorName(config.generatorType));
  uiPanel.setDifficultyName(
      DifficultySettings::fromInt(config.difficulty).getName());
  uiPanel.setTerrainEnabled(config.terrainEnabled);

  // Menu System
  MenuSystem menu;
  menu.init(window);

  // Pending size change flag
  bool pendingSizeChange = false;
  int pendingWidth = 0, pendingHeight = 0;

  // Menu callbacks - Apply settings from menu
  menu.onApply = [&]() {
    config.generatorType =
        static_cast<GeneratorType>(menu.getSelectedGenerator());
    config.solverType = static_cast<SolverType>(menu.getSelectedSolver());
    config.difficulty = menu.getDifficulty();

    // Update UI panel
    uiPanel.setGeneratorName(getGeneratorName(config.generatorType));
    uiPanel.setDifficultyName(
        DifficultySettings::fromInt(config.difficulty).getName());

    // Check if grid size changed - set pending resize
    int newWidth = menu.getGridWidth();
    int newHeight = menu.getGridHeight();
    if (newWidth != config.gridWidth || newHeight != config.gridHeight) {
      pendingSizeChange = true;
      pendingWidth = newWidth;
      pendingHeight = newHeight;
    }

    std::cout << "\nSettings applied: Generator="
              << getGeneratorName(config.generatorType)
              << ", Solver=" << getSolverName(config.solverType)
              << ", Difficulty=" << config.difficulty;
    if (pendingSizeChange) {
      std::cout << " (Grid " << pendingWidth << "x" << pendingHeight
                << " pending)";
    }
    std::cout << std::endl;
  };

  menu.onGeneratorChange = [&](int idx) {
    config.generatorType = static_cast<GeneratorType>(idx);
    uiPanel.setGeneratorName(getGeneratorName(config.generatorType));
    std::cout << "\nGenerator: " << getGeneratorName(config.generatorType)
              << std::endl;
  };

  menu.onSolverChange = [&](int idx) {
    config.solverType = static_cast<SolverType>(idx);
    std::cout << "\nSolver: " << getSolverName(config.solverType) << std::endl;
  };

  menu.onDifficultyChange = [&](int diff) {
    config.difficulty = diff;
    uiPanel.setDifficultyName(
        DifficultySettings::fromInt(config.difficulty).getName());
    std::cout << "\nDifficulty: " << config.difficulty << std::endl;
  };

  menu.onSizeChange = [&](int w, int h) {
    std::cout << "\nGrid size will change to: " << w << "x" << h << " on Apply"
              << std::endl;
  };

  // Comparison View
  ComparisonView comparisonView;
  comparisonView.init(window);

  // Solvers
  ISolver *currentSolver = nullptr;
  auto bfsSolver = std::make_unique<BFSSolver>(grid);
  auto dfsSolver = std::make_unique<DFSSolver>(grid);
  auto astarSolver = std::make_unique<AStarSolver>(grid);
  auto dijkstraSolver = std::make_unique<DijkstraSolver>(grid);
  auto greedySolver = std::make_unique<GreedySolver>(grid);
  auto bidirSolver = std::make_unique<BidirectionalBFS>(grid);

  // State
  AppState state = AppState::Idle;
  sf::Clock animationClock;
  bool mazeGenerated = false;

  // Lambda to select solver
  auto selectSolver = [&](SolverType type) -> ISolver * {
    config.solverType = type;
    switch (type) {
    case SolverType::BFS:
      return bfsSolver.get();
    case SolverType::DFS:
      return dfsSolver.get();
    case SolverType::AStar:
      return astarSolver.get();
    case SolverType::Dijkstra:
      return dijkstraSolver.get();
    case SolverType::Greedy:
      return greedySolver.get();
    case SolverType::Bidirectional:
      return bidirSolver.get();
    }
    return bfsSolver.get();
  };

  // Lambda to start solving
  auto startSolving = [&](SolverType type) {
    if (!mazeGenerated) {
      std::cout << "\nGenerate a maze first (press G or Enter)!" << std::endl;
      return;
    }
    grid->clearSolverState();
    currentSolver = selectSolver(type);
    currentSolver->init(grid->getStart(), grid->getEnd());
    state = AppState::Solving;

    // Set algorithm color
    switch (type) {
    case SolverType::BFS:
    case SolverType::Bidirectional:
      renderer.setAlgorithmType(Theme::AlgorithmType::BFS);
      break;
    case SolverType::DFS:
      renderer.setAlgorithmType(Theme::AlgorithmType::DFS);
      break;
    default:
      renderer.setAlgorithmType(Theme::AlgorithmType::BFS);
      break;
    }

    std::cout << "\nSolving with " << getSolverName(type) << "..." << std::endl;
  };

  // Lambda to start generation with current generator type
  auto startGeneration = [&](bool instant) {
    grid->clearSolverState();
    currentSolver = nullptr;
    seedManager.generateNewSeed();
    unsigned int seed = seedManager.getSeed();

    switch (config.generatorType) {
    case GeneratorType::RecursiveBacktracking:
      legacyMazeGen->startGeneration({0, 0});
      if (instant) {
        legacyMazeGen->generateFull();
        mazeGenerated = true;
        state = AppState::Idle;
      } else {
        state = AppState::Generating;
      }
      break;

    case GeneratorType::Prim:
      primGen->init(grid, seed);
      if (instant) {
        primGen->generateFull();
        mazeGenerated = true;
        state = AppState::Idle;
      } else {
        currentGenerator = std::move(primGen);
        primGen = std::make_unique<PrimGenerator>();
        state = AppState::Generating;
      }
      break;

    case GeneratorType::Kruskal:
      kruskalGen->init(grid, seed);
      if (instant) {
        kruskalGen->generateFull();
        mazeGenerated = true;
        state = AppState::Idle;
      } else {
        currentGenerator = std::move(kruskalGen);
        kruskalGen = std::make_unique<KruskalGenerator>();
        state = AppState::Generating;
      }
      break;

    case GeneratorType::BinaryTree:
      binaryGen->init(grid, seed);
      if (instant) {
        binaryGen->generateFull();
        mazeGenerated = true;
        state = AppState::Idle;
      } else {
        currentGenerator = std::move(binaryGen);
        binaryGen = std::make_unique<BinaryTreeGenerator>();
        state = AppState::Generating;
      }
      break;

    case GeneratorType::Ellers:
      ellersGen->init(grid, seed);
      if (instant) {
        ellersGen->generateFull();
        mazeGenerated = true;
        state = AppState::Idle;
      } else {
        currentGenerator = std::move(ellersGen);
        ellersGen = std::make_unique<EllersGenerator>();
        state = AppState::Generating;
      }
      break;
    }

    // Generate terrain if enabled
    if (config.terrainEnabled &&
        (instant ||
         config.generatorType == GeneratorType::RecursiveBacktracking)) {
      auto diffSettings = DifficultySettings::fromInt(config.difficulty);
      grid->generateTerrain(diffSettings.terrainDist, seed + 1);
    }

    std::cout << "\n"
              << (instant ? "Instantly generated" : "Generating")
              << " maze with " << getGeneratorName(config.generatorType)
              << " (Seed: " << seedManager.getSeedDisplay() << ")" << std::endl;
  };

  // Lambda to recreate grid with new size
  auto resizeGrid = [&](int width, int height) {
    config.gridWidth = width;
    config.gridHeight = height;

    grid = std::make_shared<Grid>(width, height);
    legacyMazeGen = std::make_unique<Maze>(grid);

    // Recreate all solvers with new grid
    bfsSolver = std::make_unique<BFSSolver>(grid);
    dfsSolver = std::make_unique<DFSSolver>(grid);
    astarSolver = std::make_unique<AStarSolver>(grid);
    dijkstraSolver = std::make_unique<DijkstraSolver>(grid);
    greedySolver = std::make_unique<GreedySolver>(grid);
    bidirSolver = std::make_unique<BidirectionalBFS>(grid);
    currentSolver = nullptr;

    // Reset generators
    primGen = std::make_unique<PrimGenerator>();
    kruskalGen = std::make_unique<KruskalGenerator>();
    binaryGen = std::make_unique<BinaryTreeGenerator>();
    ellersGen = std::make_unique<EllersGenerator>();
    currentGenerator.reset();

    renderer.setGrid(grid);
    renderer.autoCalculateCellSize();
    camera.fitToMaze(width, height, renderer.getCellSize());

    // Disable animation for large mazes
    config.animationEnabled =
        (width * height) <
        (Config::AnimationDisableThreshold * Config::AnimationDisableThreshold);

    // Update UI panel
    uiPanel.setGridSize(width, height);
    uiPanel.setAnimationEnabled(config.animationEnabled);
    uiPanel.setAlgorithmName("");

    state = AppState::Idle;
    mazeGenerated = false;

    std::cout << "\nGrid resized to " << width << "x" << height;
    if (!config.animationEnabled) {
      std::cout << " (animation disabled for performance)";
    }
    std::cout << std::endl;
  };

  // Apply pending size change after resizeGrid is defined
  if (pendingSizeChange) {
    resizeGrid(pendingWidth, pendingHeight);
    pendingSizeChange = false;
  }

  printStatus(state, config);

  // Main loop
  while (window.isOpen()) {
    // Handle pending size change from menu
    if (pendingSizeChange) {
      resizeGrid(pendingWidth, pendingHeight);
      pendingSizeChange = false;
    }

    // Event handling
    while (auto event = window.pollEvent()) {
      // Window close
      if (event->is<sf::Event::Closed>()) {
        window.close();
      }

      // Menu handles input first if visible
      if (menu.isVisible()) {
        if (auto *keyPressed = event->getIf<sf::Event::KeyPressed>()) {
          if (menu.handleKeyPress(keyPressed->code)) {
            continue;
          }
        }
      }

      // Key pressed
      if (auto *keyPressed = event->getIf<sf::Event::KeyPressed>()) {
        bool shift = keyPressed->shift;

        switch (keyPressed->code) {
        case sf::Keyboard::Key::Escape:
          if (menu.isVisible()) {
            menu.hide();
          } else if (comparisonView.isVisible()) {
            comparisonView.handleInput(sf::Keyboard::Key::Escape);
            if (!comparisonView.isVisible()) {
              state = AppState::Idle;
              std::cout << "\nComparison mode closed." << std::endl;
            }
          } else {
            window.close();
          }
          break;

        case sf::Keyboard::Key::M:
          menu.toggle();
          break;

        case sf::Keyboard::Key::G:
          startGeneration(false); // Animated
          break;

        case sf::Keyboard::Key::Enter:
          if (comparisonView.isVisible() && comparisonView.isMenuOpen()) {
            comparisonView.handleInput(sf::Keyboard::Key::Enter);
            // If comparison started, init solvers
            if (comparisonView.isRunning()) {
              comparisonView.initSolvers(grid->getStart(), grid->getEnd());
              std::cout << "\nComparison started with "
                        << comparisonView.getAlgorithmCount() << " algorithms!"
                        << std::endl;
            }
          } else {
            startGeneration(true); // Instant
          }
          break;

        case sf::Keyboard::Key::Space:
          config.animationEnabled = !config.animationEnabled;
          uiPanel.setAnimationEnabled(config.animationEnabled);
          std::cout << "\nAnimation "
                    << (config.animationEnabled ? "ENABLED" : "DISABLED")
                    << std::endl;
          break;

        case sf::Keyboard::Key::T:
          config.terrainEnabled = !config.terrainEnabled;
          if (config.terrainEnabled && mazeGenerated) {
            auto diffSettings = DifficultySettings::fromInt(config.difficulty);
            grid->generateTerrain(diffSettings.terrainDist,
                                  seedManager.getSeed() + 1);
          } else if (!config.terrainEnabled) {
            grid->clearTerrain();
          }
          std::cout << "\nTerrain "
                    << (config.terrainEnabled ? "ENABLED" : "DISABLED")
                    << std::endl;
          uiPanel.setTerrainEnabled(config.terrainEnabled);
          break;

        case sf::Keyboard::Key::H:
          uiPanel.toggleHelp();
          break;

        // Solver selection
        case sf::Keyboard::Key::B:
          startSolving(SolverType::BFS);
          break;

        case sf::Keyboard::Key::D:
          if (!shift)
            startSolving(SolverType::DFS);
          break;

        case sf::Keyboard::Key::A:
          if (!shift)
            startSolving(SolverType::AStar);
          break;

        case sf::Keyboard::Key::J:
          startSolving(SolverType::Dijkstra);
          break;

        case sf::Keyboard::Key::Y:
          startSolving(SolverType::Greedy);
          break;

        case sf::Keyboard::Key::I:
          startSolving(SolverType::Bidirectional);
          break;

        // Generator selection with Shift
        case sf::Keyboard::Key::Num1:
          if (shift) {
            config.generatorType = GeneratorType::RecursiveBacktracking;
            uiPanel.setGeneratorName(getGeneratorName(config.generatorType));
            std::cout << "\nGenerator: Recursive Backtracking" << std::endl;
          } else {
            resizeGrid(Config::PresetSmall, Config::PresetSmall);
          }
          break;

        case sf::Keyboard::Key::Num2:
          if (comparisonView.isVisible() && comparisonView.isMenuOpen()) {
            comparisonView.handleInput(keyPressed->code);
          } else if (shift) {
            config.generatorType = GeneratorType::Prim;
            uiPanel.setGeneratorName(getGeneratorName(config.generatorType));
            std::cout << "\nGenerator: Prim's Algorithm" << std::endl;
          } else {
            resizeGrid(Config::PresetMedium, Config::PresetMedium);
          }
          break;

        case sf::Keyboard::Key::Num3:
          if (shift) {
            config.generatorType = GeneratorType::Kruskal;
            uiPanel.setGeneratorName(getGeneratorName(config.generatorType));
            std::cout << "\nGenerator: Kruskal's Algorithm" << std::endl;
          } else {
            resizeGrid(Config::PresetLarge, Config::PresetLarge);
          }
          break;

        case sf::Keyboard::Key::Num4:
          if (comparisonView.isVisible() && comparisonView.isMenuOpen()) {
            comparisonView.handleInput(keyPressed->code);
          } else if (shift) {
            config.generatorType = GeneratorType::BinaryTree;
            uiPanel.setGeneratorName(getGeneratorName(config.generatorType));
            std::cout << "\nGenerator: Binary Tree" << std::endl;
          }
          break;

        case sf::Keyboard::Key::Num5:
          if (shift) {
            config.generatorType = GeneratorType::Ellers;
            uiPanel.setGeneratorName(getGeneratorName(config.generatorType));
            std::cout << "\nGenerator: Eller's Algorithm" << std::endl;
          }
          break;

        case sf::Keyboard::Key::R:
          // Reset comparison mode if running
          if (comparisonView.isRunning()) {
            comparisonView.resetSolvers();
            comparisonView.initSolvers(grid->getStart(), grid->getEnd());
          }
          grid->clearSolverState();
          if (currentSolver) {
            currentSolver->reset();
          }
          currentSolver = nullptr;
          state =
              comparisonView.isRunning() ? AppState::Comparing : AppState::Idle;
          uiPanel.setAlgorithmName("");
          break;

        case sf::Keyboard::Key::C:
          // Close and clear comparison mode
          if (comparisonView.isVisible()) {
            comparisonView.closeMenu();
          }
          grid->reset();
          grid->clearTerrain();
          legacyMazeGen->reset();
          if (currentSolver) {
            currentSolver->reset();
          }
          currentSolver = nullptr;
          currentGenerator.reset();
          state = AppState::Idle;
          mazeGenerated = false;
          uiPanel.setAlgorithmName("");
          break;

        case sf::Keyboard::Key::F:
          camera.fitToMaze(grid->getWidth(), grid->getHeight(),
                           renderer.getCellSize());
          break;

        case sf::Keyboard::Key::Equal:
        case sf::Keyboard::Key::Add:
          config.animationDelay =
              std::max(Config::MinDelay, config.animationDelay - 5);
          uiPanel.setAnimationDelay(config.animationDelay);
          break;

        case sf::Keyboard::Key::Hyphen:
        case sf::Keyboard::Key::Subtract:
          config.animationDelay =
              std::min(Config::MaxDelay, config.animationDelay + 5);
          uiPanel.setAnimationDelay(config.animationDelay);
          break;

        // Camera pan (only when menu not visible)
        case sf::Keyboard::Key::W:
          if (!menu.isVisible())
            camera.pan(0, -1);
          break;
        case sf::Keyboard::Key::Up:
          if (comparisonView.isVisible() && comparisonView.isMenuOpen()) {
            comparisonView.handleInput(keyPressed->code);
          } else if (!menu.isVisible()) {
            camera.pan(0, -1);
          }
          break;
        case sf::Keyboard::Key::S:
          if (!menu.isVisible())
            camera.pan(0, 1);
          break;
        case sf::Keyboard::Key::Down:
          if (comparisonView.isVisible() && comparisonView.isMenuOpen()) {
            comparisonView.handleInput(keyPressed->code);
          } else if (!menu.isVisible()) {
            camera.pan(0, 1);
          }
          break;
        case sf::Keyboard::Key::Left:
          if (comparisonView.isVisible() && comparisonView.isMenuOpen()) {
            comparisonView.handleInput(keyPressed->code);
          } else if (!menu.isVisible()) {
            camera.pan(-1, 0);
          }
          break;
        case sf::Keyboard::Key::Right:
          if (comparisonView.isVisible() && comparisonView.isMenuOpen()) {
            comparisonView.handleInput(keyPressed->code);
          } else if (!menu.isVisible()) {
            camera.pan(1, 0);
          }
          break;

        // Comparison mode
        case sf::Keyboard::Key::X:
          if (!comparisonView.isVisible()) {
            // Open comparison menu
            if (!mazeGenerated) {
              std::cout << "\nGenerate a maze first before comparison mode!"
                        << std::endl;
            } else {
              comparisonView.setGrid(grid);
              comparisonView.openMenu();
              state = AppState::Comparing;
              std::cout << "\nComparison menu opened. Press 2 or 4 to select "
                           "algorithm count."
                        << std::endl;
            }
          } else {
            // Let comparisonView handle X key
            if (comparisonView.handleInput(sf::Keyboard::Key::X)) {
              if (!comparisonView.isVisible()) {
                state = AppState::Idle;
                std::cout << "\nComparison mode closed." << std::endl;
              }
            }
          }
          break;

        default:
          break;
        }
      }

      // Mouse wheel for zoom
      if (auto *scroll = event->getIf<sf::Event::MouseWheelScrolled>()) {
        if (scroll->delta > 0) {
          camera.zoomIn();
        } else {
          camera.zoomOut();
        }
      }

      // Window resize
      if (auto *resized = event->getIf<sf::Event::Resized>()) {
        camera.setWindowSize(static_cast<float>(resized->size.x),
                             static_cast<float>(resized->size.y));
        sf::View view(
            sf::FloatRect({0.f, 0.f}, {static_cast<float>(resized->size.x),
                                       static_cast<float>(resized->size.y)}));
        window.setView(view);
      }
    }

    // Update logic
    if (animationClock.getElapsedTime().asMilliseconds() >=
        config.animationDelay) {
      animationClock.restart();

      switch (state) {
      case AppState::Generating:
        if (config.animationEnabled) {
          bool generating = false;

          if (config.generatorType == GeneratorType::RecursiveBacktracking) {
            generating = legacyMazeGen->step();
            if (!generating) {
              mazeGenerated = true;
              if (config.terrainEnabled) {
                auto diffSettings =
                    DifficultySettings::fromInt(config.difficulty);
                grid->generateTerrain(diffSettings.terrainDist,
                                      seedManager.getSeed() + 1);
              }
            }
          } else if (currentGenerator) {
            generating = currentGenerator->step();
            if (!generating) {
              mazeGenerated = true;
              if (config.terrainEnabled) {
                auto diffSettings =
                    DifficultySettings::fromInt(config.difficulty);
                grid->generateTerrain(diffSettings.terrainDist,
                                      seedManager.getSeed() + 1);
              }
              currentGenerator.reset();
            }
          }

          if (!generating) {
            state = AppState::Idle;
          }
        } else {
          // Generate full maze at once
          if (config.generatorType == GeneratorType::RecursiveBacktracking) {
            legacyMazeGen->generateFull();
          } else if (currentGenerator) {
            currentGenerator->generateFull();
            currentGenerator.reset();
          }
          mazeGenerated = true;
          if (config.terrainEnabled) {
            auto diffSettings = DifficultySettings::fromInt(config.difficulty);
            grid->generateTerrain(diffSettings.terrainDist,
                                  seedManager.getSeed() + 1);
          }
          state = AppState::Idle;
        }
        break;

      case AppState::Solving:
        if (currentSolver) {
          if (config.animationEnabled) {
            if (!currentSolver->step()) {
              state = AppState::Finished;
            }
          } else {
            currentSolver->solve();
            state = AppState::Finished;
          }
        }
        break;

      case AppState::Comparing:
        // Step all solvers in comparison mode
        if (comparisonView.isRunning()) {
          if (config.animationEnabled) {
            comparisonView.stepAll();
          } else {
            // Run all solvers to completion
            while (!comparisonView.allFinished()) {
              comparisonView.stepAll();
            }
          }
        }
        break;

      default:
        break;
      }

      printStatus(state, config, currentSolver);
    }

    // Update UI panel
    switch (state) {
    case AppState::Idle:
      uiPanel.setState(mazeGenerated ? "Ready" : "Generate maze (G)");
      uiPanel.setAppState(maze::AppState::Idle);
      break;
    case AppState::Generating:
      uiPanel.setState("Generating...");
      uiPanel.setAppState(maze::AppState::Generating);
      break;
    case AppState::Solving:
      uiPanel.setState("Solving...");
      uiPanel.setAppState(maze::AppState::Solving);
      break;
    case AppState::Finished:
      uiPanel.setState(currentSolver && currentSolver->foundPath()
                           ? "Path Found!"
                           : "No Path");
      uiPanel.setAppState(maze::AppState::Idle); // Finished = back to idle UI
      break;
    case AppState::Comparing:
      uiPanel.setState("Comparing...");
      uiPanel.setAppState(maze::AppState::Comparing);
      break;
    }

    // Update algorithm stats
    if (currentSolver) {
      auto stats = currentSolver->getStats();
      uiPanel.setAlgorithmName(currentSolver->getName());
      uiPanel.setNodesVisited(stats.nodesVisited);
      uiPanel.setQueueSize(stats.nodesInQueue);
      uiPanel.setPathLength(stats.pathLength);
      uiPanel.setElapsedTime(stats.elapsedTime);

      // Calculate and set path cost if terrain enabled and path found
      if (config.terrainEnabled && currentSolver->foundPath()) {
        const auto &path = currentSolver->getPath();
        float totalCost = 0.0f;
        for (size_t i = 0; i + 1 < path.size(); ++i) {
          totalCost += grid->getMovementCost(path[i], path[i + 1]);
        }
        uiPanel.setPathCost(totalCost);
      } else {
        uiPanel.setPathCost(0.0f);
      }
    }

    // Update FPS
    uiPanel.setFPS(renderer.getStats().fps);

    // Render
    renderer.clear();

    // Render comparison view if visible (takes over the screen)
    if (comparisonView.isVisible()) {
      if (comparisonView.isRunning()) {
        // Running comparison - don't show base maze, only split grids
        uiPanel.render(window);
        comparisonView.render(window);
      } else {
        // Menu open - show normal view behind menu
        renderer.render();
        if (config.terrainEnabled) {
          renderer.renderTerrain();
        }
        uiPanel.render(window);
        comparisonView.render(window);
      }
    } else {
      renderer.render();

      // Render terrain colors if enabled
      if (config.terrainEnabled) {
        renderer.renderTerrain();
      }

      uiPanel.render(window);
      menu.render(window);
    }

    renderer.display();
  }

  std::cout << "\n\nGoodbye!\n";
  return 0;
}
