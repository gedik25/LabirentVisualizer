#pragma once

#include <SFML/Graphics/Color.hpp>
#include <cstdint>
#include <string>
#include <array>

namespace maze {

/**
 * Terrain types for maze cells.
 * Each terrain type has a movement cost multiplier.
 */
enum class TerrainType : uint8_t {
    Normal = 0,   // Base terrain
    Grass,        // Slight slowdown
    Sand,         // Moderate slowdown
    Water,        // Significant slowdown
    Mud,          // Major slowdown
    Ice,          // Speed boost
    Lava,         // Extreme slowdown (almost impassable)
    Count         // Number of terrain types
};

/**
 * Terrain properties and utilities.
 */
struct TerrainInfo {
    const char* name;
    float cost;         // Movement cost multiplier
    sf::Color color;    // Visualization color
    
    // Whether this terrain should be avoided in easy mode
    bool isHazard() const { return cost >= 2.0f; }
};

// Terrain data table
constexpr size_t TERRAIN_COUNT = static_cast<size_t>(TerrainType::Count);

inline const TerrainInfo& getTerrainInfo(TerrainType type) {
    static const std::array<TerrainInfo, TERRAIN_COUNT> terrainData = {{
        {"Normal",  1.0f,  sf::Color{30, 30, 30}},       // Dark grey
        {"Grass",   1.2f,  sf::Color{34, 139, 34}},      // Forest green
        {"Sand",    1.5f,  sf::Color{194, 178, 128}},    // Sand/tan
        {"Water",   2.0f,  sf::Color{30, 144, 255}},     // Dodger blue
        {"Mud",     3.0f,  sf::Color{101, 67, 33}},      // Brown
        {"Ice",     0.8f,  sf::Color{173, 216, 230}},    // Light blue
        {"Lava",    10.0f, sf::Color{255, 69, 0}}        // Orange red
    }};
    
    size_t idx = static_cast<size_t>(type);
    if (idx >= TERRAIN_COUNT) idx = 0;
    return terrainData[idx];
}

inline float getTerrainCost(TerrainType type) {
    return getTerrainInfo(type).cost;
}

inline const char* getTerrainName(TerrainType type) {
    return getTerrainInfo(type).name;
}

inline sf::Color getTerrainColor(TerrainType type) {
    return getTerrainInfo(type).color;
}

/**
 * Terrain distribution settings.
 * Defines the probability of each terrain type being placed.
 */
struct TerrainDistribution {
    float normal = 0.40f;   // 40%
    float grass = 0.15f;    // 15%
    float sand = 0.15f;     // 15%
    float water = 0.10f;    // 10%
    float mud = 0.10f;      // 10%
    float ice = 0.05f;      // 5%
    float lava = 0.05f;     // 5%
    
    // Cluster settings
    float clusterChance = 0.3f;    // Chance to extend cluster
    int maxClusterSize = 10;       // Maximum cluster size
    
    // Preset distributions
    static TerrainDistribution balanced() {
        return TerrainDistribution{};
    }
    
    static TerrainDistribution watery() {
        TerrainDistribution d;
        d.normal = 0.30f;
        d.water = 0.40f;
        d.ice = 0.15f;
        d.grass = 0.10f;
        d.sand = 0.05f;
        d.mud = 0.0f;
        d.lava = 0.0f;
        return d;
    }
    
    static TerrainDistribution swampy() {
        TerrainDistribution d;
        d.normal = 0.20f;
        d.mud = 0.35f;
        d.water = 0.20f;
        d.grass = 0.20f;
        d.sand = 0.0f;
        d.ice = 0.0f;
        d.lava = 0.05f;
        return d;
    }
    
    static TerrainDistribution volcanic() {
        TerrainDistribution d;
        d.normal = 0.40f;
        d.lava = 0.30f;
        d.sand = 0.20f;
        d.grass = 0.0f;
        d.water = 0.0f;
        d.mud = 0.10f;
        d.ice = 0.0f;
        return d;
    }
    
    static TerrainDistribution winter() {
        TerrainDistribution d;
        d.normal = 0.30f;
        d.ice = 0.40f;
        d.water = 0.15f;
        d.grass = 0.0f;
        d.sand = 0.0f;
        d.mud = 0.10f;
        d.lava = 0.05f;
        return d;
    }
    
    // Get probability array
    std::array<float, TERRAIN_COUNT> toArray() const {
        return {normal, grass, sand, water, mud, ice, lava};
    }
};

} // namespace maze
