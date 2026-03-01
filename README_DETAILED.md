# Technical Architecture & Implementation Deep-Dive
## Labirent Visualizer (C++ / SFML)

This document provides a highly technical, rigorous exploration of the mathematical models, algorithms, and system architecture underlying the **Labirent Visualizer** application. It serves as an architectural blueprint for engineers looking to understand the mechanics of the custom pathfinding rendering engine, matrix calculations, and state-machine integrations.

---

## 1. System Architecture

The application adopts a decoupled Object-Oriented paradigm, aggressively separating Core mathematical logic (maze generation, grid data structures, pathfinding algorithms) from UI/Rendering loops (SFML graphical displays, matrices, viewports).

### Core Components
- **`Grid`**: The central data structure representing the environment topology. Handles coordinate-based bounds checking, neighbor resolution, and bitwise cellular flag management (`CellFlags::InPath`, `CellFlags::Current`, `TerrainType`, etc.).
- **`IMazeGenerator` / `ISolver`**: Interfaces enforcing a tick-based stepping mechanism. Examples like `PrimGenerator`, `BinaryTreeGenerator`, or `AStarSolver` do not "solve" in a single blocking `while(true)` loop. Instead, they expose a stateful `step()` method. This permits the main application loop to poll the visualizer thread without freezing.
- **`Camera` & `ComparisonView`**: Abstractions over SFML's 2D View properties. They encapsulate complex viewport clipping arrays, affine transformations, zoom scalar logic, and dynamic sub-panel mapping.

---

## 2. Grid & Terrain Mechanics

### Transition to Block Wall Topology
Historically, classical maze algorithms interpret "walls" simply as edges (boolean borders) traveling between zero-dimension vertices. The Labirent Visualizer translates these abstractions into a *Block Wall Architecture*. 

A standard $W \times H$ logical maze is expanded into a $(2W + 1) \times (2H + 1)$ physical tilemap array. 
- Even-coordinate pairs $(x_{even}, y_{even})$ serve as fixed structural pillars.
- Odd/Even pairs act as destructible passage bounds.
- Odd-coordinate pairs $(x_{odd}, y_{odd})$ denote the primary walkable cells.

This uniform volumetric approach allows standard SFML `sf::RectangleShape` draw calls to treat walls and passages as identically sized entities, dramatically simplifying physics overlaps, coordinate bounding queries, and pixel-aligned clipping.

### Terrain Mode Weights
Terrain introduces distinct traversal cost scalars mapped to walkable coordinate cells. By default, cells utilize $Cost = 1.0f$. When Terrain Mode is activated:
* **Grass**: Base cost $\times \ 1.2$
* **Water**: Base cost $\times \ 2.0$
* **Lava**: Base cost $\times \ 10.0$

For algorithmic uniformity passing cell-to-cell, the movement cost dynamically averages out:
$$C_{movement}(a, b) = \frac{Cost(a) + Cost(b)}{2.0f}$$

---

## 3. Algorithms Breakdown

### Maze Generation
- **Recursive Backtracking**: Implements a randomized Depth-First Search approach over the grid. It utilizes a `std::stack` for managing node memory. Its time complexity is bounded by $O(V + E)$ where $V$ is total walkable grid cells. Space complexity hits $O(V)$ in the worst-case unbranched tree.
- **Prim's Algorithm**: Utilizes a randomized edge pool (queue), constantly selecting minimum-weight bounding limits to expand a unified spanning tree organically.

### Pathfinding Implementations
All algorithms track structural exploration sets: $Fringe$ (Queued), $Visited$ (Evaluated) and maintain backwards linking trees (`std::unordered_map<Position, Position>`) corresponding to their search trails.

1. **BFS (Breadth-First Search)**: A `std::queue` FIFO approach. Unweighted; inherently locates the shortest topological path in uniform environments.
2. **DFS (Depth-First Search)**: A `std::stack` LIFO methodology. Blindest traversal model, prioritizing exhaustive linear descents.
3. **Dijkstra's Algorithm**: Uses a `std::priority_queue` Min-Heap evaluated entirely on $g(n)$—the cumulative exact cost measured utilizing traversing $C_{movement}$. Naturally adapts avoiding Lava loops preferring longer physical stretches of Ice/Normal tiles.
4. **A***: Expands Dijkstra via a heuristic estimate $h(n)$. The implementation utilizes **Manhattan Distance** ($|x_1 - x_2| + |y_1 - y_2|$) as grids limit eight-axis or diagonal freedom. Total heuristic score maps to $f(n) = g(n) + h(n)$.

---

## 4. Rendering & Camera Math

The C++ SFML back-end requires discrete pixel boundaries handled iteratively at 60Hz.

### Smooth Dynamic Auto-Scaling (Zoom)
Input mapping from precision trackpads or smooth-scrolling mice yields floating-point delta events. Rather than stepping discretely (e.g. `1.1f` sizes), the camera system utilizes exact derivatives:
```cpp
float zoomFactor = 1.0f - (scroll->delta * 0.1f);
zoomFactor = std::max(zoomFactor, 0.1f); // Clamp physics inversion
camera.zoom(zoomFactor);
```

### The 'F' (Fit-to-Screen) Sub-Panel Matrix
In **Comparison Mode**, the window subdivides into distinct concurrent viewports. The mathematical goal is maintaining geometric aspect ratios across all 4 algorithmic viewports irrespective of grid shapes.

The formula resolves maximum fitting width ($W_{available}$) and height ($H_{available}$) by dynamically clipping $10px$ logic margins and $35px$ typography header offsets per sub-panel.

```math
Scale_{x} = \frac{W_{available} \times 0.9}{W_{virtualMaze}} \\
Scale_{y} = \frac{H_{available} \times 0.9}{H_{virtualMaze}}
```
It extracts the bounding minimum `m_zoom = std::min(Scale_x, Scale_y)` maintaining uniform scale. Combined with automated pan offsets converging at zero $(0.0f, 0.0f)$, `sf::View::setCenter` implicitly snaps the generated maze tile matrix dead-center into the bounded sub-viewport. 

---

## 5. Performance & Optimization

- **State-based Animation**: To animate $10^5$ pathfinding nodes concurrently, conventional `std::this_thread::sleep_for` inside while loops will stall `sf::RenderWindow::pollEvent`. To conquer this, solvers manage a discrete internal `step()` instruction set. The primary application loop queries `animationClock.getElapsedTime().asMilliseconds() >= config.animationDelay` strictly on the main thread, unspooling node execution non-destructively.
- **SFML Resource Handling**: Repeated text construction or geometry allocations in 60Hz loop cycles generates profound GC-equivalent jitter in C++. We employ static `sf::RectangleShape` allocations directly bound within `Renderer` class lifecycles—translating strictly by `setPosition()` pointers instead of initiating dynamic heap geometries per cell frame.
- **Immediate Vector Offsets**: Heavy lookup systems utilize flat arrays $f(x, y) = y \times width + x$ instead of complex nested vectors `std::vector<std::vector<T>>` to maximize CPU cache continuity and L1-burst loading ratios.
