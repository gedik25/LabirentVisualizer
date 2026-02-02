#include <SFML/Graphics.hpp>
#include <iostream>
#include <memory>
#include <chrono>
#include <thread>

#include "Core/Grid.hpp"
#include "Core/Maze.hpp"
#include "Algorithms/BFSSolver.hpp"
#include "Algorithms/DFSSolver.hpp"
#include "Visualization/Camera.hpp"
#include "Visualization/Renderer.hpp"
#include "Visualization/Theme.hpp"

using namespace maze;

// Application state
enum class AppState {
    Idle,           // Waiting for user input
    Generating,     // Generating maze
    Solving,        // Running pathfinding
    Finished        // Solution found/not found
};

// Configuration
struct Config {
    int gridWidth = 25;
    int gridHeight = 25;
    int animationDelay = 10;  // milliseconds between steps
    bool animationEnabled = true;
    
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
              << "G        - Generate new maze (animated)\n"
              << "Enter    - Generate new maze (instant)\n"
              << "B        - Solve with BFS (Breadth-First Search)\n"
              << "D        - Solve with DFS (Depth-First Search)\n"
              << "Space    - Toggle animation ON/OFF\n"
              << "R        - Reset (clear solution, keep maze)\n"
              << "C        - Clear all (reset to empty grid)\n"
              << "+/-      - Adjust animation speed\n"
              << "WASD     - Pan camera\n"
              << "Scroll   - Zoom in/out\n"
              << "F        - Fit maze to window\n"
              << "1        - Small maze (25x25)\n"
              << "2        - Medium maze (100x100)\n"
              << "3        - Large maze (500x500)\n"
              << "ESC      - Exit\n"
              << "================================\n\n";
}

void printStatus(AppState state, const Config& config, ISolver* solver = nullptr) {
    std::cout << "\r";  // Return to start of line
    std::cout << "[" << config.gridWidth << "x" << config.gridHeight << "] ";
    std::cout << "Anim: " << (config.animationEnabled ? "ON " : "OFF") << " | ";
    std::cout << "Delay: " << config.animationDelay << "ms | ";
    
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
    }
    std::cout << std::flush;
}

int main() {
    // Print help at startup
    printHelp();
    
    // Configuration
    Config config;
    
    // Create window
    const unsigned int windowWidth = 1200;
    const unsigned int windowHeight = 800;
    
    sf::RenderWindow window(
        sf::VideoMode({windowWidth, windowHeight}),
        "Maze Visualizer - Labirent",
        sf::Style::Default
    );
    window.setFramerateLimit(60);
    
    // Initialize components
    auto grid = std::make_shared<Grid>(config.gridWidth, config.gridHeight);
    auto mazeGen = std::make_unique<Maze>(grid);
    
    Camera camera(static_cast<float>(windowWidth), static_cast<float>(windowHeight));
    Renderer renderer(window, camera);
    renderer.setGrid(grid);
    renderer.autoCalculateCellSize();
    camera.fitToMaze(grid->getWidth(), grid->getHeight(), renderer.getCellSize());
    
    // Solvers
    ISolver* currentSolver = nullptr;
    auto bfsSolver = std::make_unique<BFSSolver>(grid);
    auto dfsSolver = std::make_unique<DFSSolver>(grid);
    
    // State
    AppState state = AppState::Idle;
    sf::Clock animationClock;
    
    // Lambda to recreate grid with new size
    auto resizeGrid = [&](int width, int height) {
        config.gridWidth = width;
        config.gridHeight = height;
        
        grid = std::make_shared<Grid>(width, height);
        mazeGen = std::make_unique<Maze>(grid);
        bfsSolver = std::make_unique<BFSSolver>(grid);
        dfsSolver = std::make_unique<DFSSolver>(grid);
        currentSolver = nullptr;
        
        renderer.setGrid(grid);
        renderer.autoCalculateCellSize();
        camera.fitToMaze(width, height, renderer.getCellSize());
        
        // Disable animation for large mazes
        config.animationEnabled = (width * height) < 
            (Config::AnimationDisableThreshold * Config::AnimationDisableThreshold);
        
        state = AppState::Idle;
        std::cout << "\nGrid resized to " << width << "x" << height;
        if (!config.animationEnabled) {
            std::cout << " (animation disabled for performance)";
        }
        std::cout << std::endl;
    };
    
    printStatus(state, config);
    
    // Main loop
    while (window.isOpen()) {
        // Event handling - SFML 3.x style
        while (auto event = window.pollEvent()) {
            // Window close
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }
            
            // Key pressed
            if (auto* keyPressed = event->getIf<sf::Event::KeyPressed>()) {
                switch (keyPressed->code) {
                    case sf::Keyboard::Key::Escape:
                        window.close();
                        break;
                        
                    case sf::Keyboard::Key::G:
                        // Generate new maze (animated)
                        grid->clearSolverState();
                        mazeGen->startGeneration({0, 0});
                        currentSolver = nullptr;
                        state = AppState::Generating;
                        renderer.setAlgorithmType(Theme::AlgorithmType::BFS);
                        break;
                        
                    case sf::Keyboard::Key::Enter:
                        // Generate new maze instantly (no animation)
                        grid->clearSolverState();
                        mazeGen->startGeneration({0, 0});
                        mazeGen->generateFull();
                        currentSolver = nullptr;
                        state = AppState::Idle;
                        renderer.setAlgorithmType(Theme::AlgorithmType::BFS);
                        std::cout << "\nMaze generated instantly!" << std::endl;
                        break;
                        
                    case sf::Keyboard::Key::Space:
                        // Toggle animation
                        config.animationEnabled = !config.animationEnabled;
                        std::cout << "\nAnimation " << (config.animationEnabled ? "ENABLED" : "DISABLED") << std::endl;
                        break;
                        
                    case sf::Keyboard::Key::B:
                        // Solve with BFS
                        if (mazeGen->isComplete()) {
                            grid->clearSolverState();
                            bfsSolver->init(grid->getStart(), grid->getEnd());
                            currentSolver = bfsSolver.get();
                            state = AppState::Solving;
                            renderer.setAlgorithmType(Theme::AlgorithmType::BFS);
                        }
                        break;
                        
                    case sf::Keyboard::Key::D:
                        // Solve with DFS
                        if (mazeGen->isComplete()) {
                            grid->clearSolverState();
                            dfsSolver->init(grid->getStart(), grid->getEnd());
                            currentSolver = dfsSolver.get();
                            state = AppState::Solving;
                            renderer.setAlgorithmType(Theme::AlgorithmType::DFS);
                        }
                        break;
                        
                    case sf::Keyboard::Key::R:
                        // Reset solution only
                        grid->clearSolverState();
                        if (currentSolver) {
                            currentSolver->reset();
                        }
                        state = AppState::Idle;
                        break;
                        
                    case sf::Keyboard::Key::C:
                        // Clear everything
                        grid->reset();
                        mazeGen->reset();
                        if (currentSolver) {
                            currentSolver->reset();
                        }
                        currentSolver = nullptr;
                        state = AppState::Idle;
                        break;
                        
                    case sf::Keyboard::Key::F:
                        // Fit to window
                        camera.fitToMaze(grid->getWidth(), grid->getHeight(), 
                                        renderer.getCellSize());
                        break;
                        
                    case sf::Keyboard::Key::Num1:
                        resizeGrid(Config::PresetSmall, Config::PresetSmall);
                        break;
                        
                    case sf::Keyboard::Key::Num2:
                        resizeGrid(Config::PresetMedium, Config::PresetMedium);
                        break;
                        
                    case sf::Keyboard::Key::Num3:
                        resizeGrid(Config::PresetLarge, Config::PresetLarge);
                        break;
                        
                    case sf::Keyboard::Key::Equal:  // + key
                    case sf::Keyboard::Key::Add:
                        config.animationDelay = std::max(Config::MinDelay, 
                                                         config.animationDelay - 5);
                        break;
                        
                    case sf::Keyboard::Key::Hyphen:  // - key
                    case sf::Keyboard::Key::Subtract:
                        config.animationDelay = std::min(Config::MaxDelay, 
                                                         config.animationDelay + 5);
                        break;
                        
                    // Camera pan
                    case sf::Keyboard::Key::W:
                    case sf::Keyboard::Key::Up:
                        camera.pan(0, -1);
                        break;
                    case sf::Keyboard::Key::S:
                    case sf::Keyboard::Key::Down:
                        camera.pan(0, 1);
                        break;
                    case sf::Keyboard::Key::A:
                    case sf::Keyboard::Key::Left:
                        camera.pan(-1, 0);
                        break;
                    case sf::Keyboard::Key::Right:
                        camera.pan(1, 0);
                        break;
                        
                    default:
                        break;
                }
            }
            
            // Mouse wheel for zoom
            if (auto* scroll = event->getIf<sf::Event::MouseWheelScrolled>()) {
                if (scroll->delta > 0) {
                    camera.zoomIn();
                } else {
                    camera.zoomOut();
                }
            }
            
            // Window resize
            if (auto* resized = event->getIf<sf::Event::Resized>()) {
                camera.setWindowSize(static_cast<float>(resized->size.x),
                                    static_cast<float>(resized->size.y));
                sf::View view(sf::FloatRect({0.f, 0.f}, 
                    {static_cast<float>(resized->size.x), 
                     static_cast<float>(resized->size.y)}));
                window.setView(view);
            }
        }
        
        // Update logic
        if (animationClock.getElapsedTime().asMilliseconds() >= config.animationDelay) {
            animationClock.restart();
            
            switch (state) {
                case AppState::Generating:
                    if (config.animationEnabled) {
                        if (!mazeGen->step()) {
                            state = AppState::Idle;
                        }
                    } else {
                        // Generate full maze at once for large mazes
                        mazeGen->generateFull();
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
                            // Solve at once for large mazes
                            currentSolver->solve();
                            state = AppState::Finished;
                        }
                    }
                    break;
                    
                default:
                    break;
            }
            
            printStatus(state, config, currentSolver);
        }
        
        // Render
        renderer.clear();
        renderer.render();
        renderer.display();
    }
    
    std::cout << "\n\nGoodbye!\n";
    return 0;
}
