# Labirent - Maze Generation and Pathfinding Visualizer

A high-performance maze generation and pathfinding visualization tool built with C++ and SFML 3.x. Compare multiple algorithms, explore different terrain types, and analyze pathfinding performance.

## Features

### Pathfinding Algorithms
- **BFS** (Breadth-First Search) - Guarantees shortest path in unweighted graphs
- **DFS** (Depth-First Search) - Memory efficient exploration
- **A*** (A-Star) - Optimal pathfinding with heuristic guidance
- **Dijkstra** - Shortest path for weighted graphs
- **Greedy Best-First** - Fast but non-optimal heuristic search
- **Bidirectional BFS** - Searches from both ends simultaneously

### Maze Generation Algorithms
- **Recursive Backtracking** - Classic DFS-based generation
- **Prim's Algorithm** - Grows maze from random cell using frontier
- **Kruskal's Algorithm** - Union-Find based random wall removal
- **Binary Tree** - Simple and fast with diagonal bias
- **Eller's Algorithm** - Row-by-row generation with O(width) memory

### Terrain System
| Terrain | Cost | Color |
|---------|------|-------|
| Normal | 1.0x | Dark Grey |
| Grass | 1.2x | Forest Green |
| Sand | 1.5x | Tan |
| Water | 2.0x | Blue |
| Mud | 3.0x | Brown |
| Ice | 0.8x | Light Blue |
| Lava | 10.0x | Orange Red |

### Additional Features
- **Scalable Grid**: Supports sizes from 25x25 to 10000x10000
- **Comparison Mode**: Compare 2-4 algorithms side-by-side on the same maze
- **Seed System**: Reproducible mazes with seed display and input
- **Difficulty Levels**: 5 levels (Very Easy to Very Hard) affecting terrain distribution
- **Terrain Presets**: Balanced, Watery World, Muddy Swamp, Volcanic, Winter
- **Interactive Menu**: Arrow key navigation for all settings
- **Real-time Visualization**: Watch algorithms explore the maze
- **Camera Controls**: Pan and zoom for large mazes
- **Dark Theme**: Modern dark mode with neon accent colors

## Screenshots

Color scheme:
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

### Main Controls
| Key | Action |
|-----|--------|
| M | Open/Close Menu |
| G | Generate new maze |
| Enter | Generate maze instantly (no animation) |
| Space | Toggle animation on/off |
| R | Reset solution (keep maze) |
| C | Clear all |
| H | Toggle help panel |
| ESC | Exit |

### Algorithm Selection
| Key | Action |
|-----|--------|
| B | Solve with BFS |
| D | Solve with DFS |
| A | Solve with A* |
| J | Solve with Dijkstra |
| Y | Solve with Greedy Best-First |
| I | Solve with Bidirectional BFS |

### Camera & Display
| Key | Action |
|-----|--------|
| WASD / Arrows | Pan camera |
| Mouse Scroll | Zoom in/out |
| F | Fit maze to window |
| +/- | Adjust animation speed |

### Grid Size Presets
| Key | Action |
|-----|--------|
| 1 | Small maze (25x25) |
| 2 | Medium maze (100x100) |
| 3 | Large maze (500x500) |

### Menu Navigation
| Key | Action |
|-----|--------|
| Up/Down | Navigate items |
| Left/Right | Change option value |
| Enter | Select/Apply |
| Esc | Close menu |

## Project Structure

```
Labirent/
├── CMakeLists.txt
├── README.md
├── src/
│   ├── main.cpp
│   ├── Core/
│   │   ├── Cell.hpp              # Cell state flags
│   │   ├── Grid.hpp/cpp          # Memory-efficient grid with terrain
│   │   ├── Maze.hpp/cpp          # Legacy maze generation
│   │   ├── Terrain.hpp           # Terrain types and costs
│   │   ├── DifficultySettings.hpp # Difficulty presets
│   │   └── SeedManager.hpp       # Seed management
│   ├── Algorithms/
│   │   ├── ISolver.hpp           # Solver interface
│   │   ├── BFSSolver.hpp/cpp     # BFS implementation
│   │   ├── DFSSolver.hpp/cpp     # DFS implementation
│   │   ├── AStarSolver.hpp/cpp   # A* implementation
│   │   ├── DijkstraSolver.hpp/cpp # Dijkstra implementation
│   │   ├── GreedySolver.hpp/cpp  # Greedy Best-First
│   │   └── BidirectionalBFS.hpp/cpp # Bidirectional BFS
│   ├── Generators/
│   │   ├── IMazeGenerator.hpp    # Generator interface
│   │   ├── PrimGenerator.hpp/cpp # Prim's algorithm
│   │   ├── KruskalGenerator.hpp/cpp # Kruskal's algorithm
│   │   ├── BinaryTreeGenerator.hpp/cpp # Binary Tree
│   │   └── EllersGenerator.hpp/cpp # Eller's algorithm
│   ├── Visualization/
│   │   ├── Theme.hpp             # Color scheme
│   │   ├── Camera.hpp/cpp        # Pan/Zoom controls
│   │   ├── Renderer.hpp/cpp      # Drawing with culling
│   │   └── UIPanel.hpp/cpp       # Status and stats display
│   └── UI/
│       ├── MenuSystem.hpp/cpp    # Interactive menu
│       └── ComparisonView.hpp/cpp # Algorithm comparison
└── build/
```

## Architecture

The project follows a modular architecture:

1. **Core Module**: Grid data structure with memory-efficient storage (1 byte per cell using bit flags) + terrain data
2. **Algorithms Module**: Extensible solver interface with 6 pathfinding algorithms
3. **Generators Module**: Multiple maze generation algorithms with common interface
4. **Visualization Module**: Efficient rendering with viewport culling for large mazes
5. **UI Module**: Menu system and comparison view

## Algorithm Comparison

The comparison mode allows you to run multiple algorithms simultaneously on the same maze:

- Select 2-4 algorithms to compare
- All algorithms run on identical maze copies
- Results table shows: Visited nodes, Path length, Execution time
- Best values are highlighted in green

## Terrain and Weighted Pathfinding

A* and Dijkstra algorithms use terrain costs for weighted pathfinding:
- Movement cost = average of source and destination terrain costs
- Ice terrain provides a speed boost (0.8x cost)
- Lava is nearly impassable (10x cost)

## Performance Notes

- Grids up to 500x500: Full animation support
- Grids 500-2000: Animation optional, viewport culling active
- Grids >2000: Animation disabled, instant results only

## License

MIT License - Feel free to use and modify for educational purposes.

## Author

Created as a Computer Engineering educational project for learning:
- Graph algorithms (BFS, DFS, A*, Dijkstra)
- Data structures (Queue, Stack, Priority Queue, Union-Find)
- Maze generation algorithms
- Memory optimization techniques
- Real-time graphics programming
- Weighted graph pathfinding
