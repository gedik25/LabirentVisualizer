cmake_minimum_required(VERSION 3.16)
project(LabirentVisualizer VERSION 1.0.0 LANGUAGES CXX)

# C++17 standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Compiler flags
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -Wall -Wextra -Wpedantic")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")

# Find SFML 3.x (installed via Homebrew)
find_package(SFML 3 COMPONENTS Graphics Window System REQUIRED)

# Source files
set(SOURCES
    src/main.cpp
    src/Core/Grid.cpp
    src/Core/Maze.cpp
    src/Algorithms/BFSSolver.cpp
    src/Algorithms/DFSSolver.cpp
    src/Algorithms/AStarSolver.cpp
    src/Algorithms/DijkstraSolver.cpp
    src/Algorithms/GreedySolver.cpp
    src/Algorithms/BidirectionalBFS.cpp
    src/Visualization/Camera.cpp
    src/Visualization/Renderer.cpp
    src/Visualization/UIPanel.cpp
)

# Header files (for IDE support)
set(HEADERS
    src/Core/Cell.hpp
    src/Core/Grid.hpp
    src/Core/Maze.hpp
    src/Algorithms/ISolver.hpp
    src/Algorithms/BFSSolver.hpp
    src/Algorithms/DFSSolver.hpp
    src/Algorithms/AStarSolver.hpp
    src/Algorithms/DijkstraSolver.hpp
    src/Algorithms/GreedySolver.hpp
    src/Algorithms/BidirectionalBFS.hpp
    src/Visualization/Theme.hpp
    src/Visualization/Camera.hpp
    src/Visualization/Renderer.hpp
    src/Visualization/UIPanel.hpp
)

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/src)

# Link SFML libraries
target_link_libraries(${PROJECT_NAME} PRIVATE SFML::Graphics SFML::Window SFML::System)

# macOS specific settings
if(APPLE)
    # Use rpath for finding libraries
    set_target_properties(${PROJECT_NAME} PROPERTIES
        INSTALL_RPATH "@executable_path/../lib"
        BUILD_WITH_INSTALL_RPATH TRUE
    )
endif()

# Output directory
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
