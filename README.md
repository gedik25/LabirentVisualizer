# Labirent - Maze Generation and Pathfinding Visualizer

A high-performance maze generation and pathfinding visualization tool built with C++ and SFML 3.x.

## Features

- **Maze Generation**: Recursive Backtracking algorithm with step-by-step animation
- **Pathfinding Algorithms**: 
  - BFS (Breadth-First Search) - Guarantees shortest path
  - DFS (Depth-First Search) - Memory efficient exploration
- **Scalable Grid**: Supports sizes from 25x25 to 10000x10000
- **Real-time Visualization**: Watch algorithms explore the maze
- **Camera Controls**: Pan and zoom for large mazes
- **Dark Theme**: Modern dark mode with neon accent colors

## Screenshots

The visualizer uses a color scheme:
- **Cyan (#00E5FF)**: BFS visited cells
- **Yellow (#FFD600)**: DFS visited cells
- **Magenta (#FF00FF)**: Solution path
- **Green**: Start point
- **Red**: End point

## Requirements

- C++17 compatible compiler
- CMake 3.16+
- SFML 3.x

### macOS Installation

```bash
# Install dependencies via Homebrew
brew install cmake sfml
```

## Building

```bash
# Clone or navigate to project directory
cd Labirent

# Create build directory
mkdir build && cd build

# Configure and build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . -j8

# Run
./bin/LabirentVisualizer
```

## Controls

| Key | Action |
|-----|--------|
| G | Generate new maze |
| B | Solve with BFS |
| D | Solve with DFS |
| R | Reset solution (keep maze) |
| C | Clear all |
| +/- | Adjust animation speed |
| WASD / Arrows | Pan camera |
| Mouse Scroll | Zoom in/out |
| F | Fit maze to window |
| 1 | Small maze (25x25) |
| 2 | Medium maze (100x100) |
| 3 | Large maze (500x500) |
| ESC | Exit |

## Project Structure

```
Labirent/
├── CMakeLists.txt
├── README.md
├── src/
│   ├── main.cpp
│   ├── Core/
│   │   ├── Cell.hpp          # Cell state flags
│   │   ├── Grid.hpp/cpp      # Memory-efficient grid
│   │   └── Maze.hpp/cpp      # Maze generation
│   ├── Algorithms/
│   │   ├── ISolver.hpp       # Solver interface
│   │   ├── BFSSolver.hpp/cpp # BFS implementation
│   │   └── DFSSolver.hpp/cpp # DFS implementation
│   └── Visualization/
│       ├── Theme.hpp         # Color scheme
│       ├── Camera.hpp/cpp    # Pan/Zoom controls
│       └── Renderer.hpp/cpp  # Drawing with culling
└── build/
```

## Architecture

The project follows a modular architecture:

1. **Core Module**: Grid data structure with memory-efficient storage (1 byte per cell using bit flags)
2. **Algorithms Module**: Extensible solver interface - add new algorithms by implementing `ISolver`
3. **Visualization Module**: Efficient rendering with viewport culling for large mazes

## Adding New Algorithms

To add a new pathfinding algorithm (e.g., A*):

1. Create `AStarSolver.hpp` and `AStarSolver.cpp` in `src/Algorithms/`
2. Inherit from `SolverBase` and implement required methods
3. Add to `CMakeLists.txt` sources
4. Instantiate in `main.cpp` and add keyboard shortcut

Example:
```cpp
class AStarSolver : public SolverBase {
public:
    explicit AStarSolver(std::shared_ptr<Grid> grid);
    void init(const Position& start, const Position& end) override;
    bool step() override;
    bool solve() override;
    std::string getName() const override { return "A* Search"; }
    // ... implement other methods
};
```

## Performance Notes

- Grids up to 500x500: Full animation support
- Grids 500-2000: Animation optional, viewport culling active
- Grids >2000: Animation disabled, instant results only

## License

MIT License - Feel free to use and modify for educational purposes.

## Author

Created as a Computer Engineering educational project for learning:
- Graph algorithms (BFS, DFS)
- Data structures (Queue, Stack)
- Memory optimization techniques
- Real-time graphics programming
