#pragma once

#include "Terrain.hpp"
#include <string>
#include <array>

namespace maze {

/**
 * Difficulty level enum (1-5 scale).
 */
enum class DifficultyLevel : int {
    VeryEasy = 1,
    Easy = 2,
    Medium = 3,
    Hard = 4,
    VeryHard = 5
};

/**
 * Difficulty settings that affect maze generation and terrain.
 */
struct DifficultySettings {
    DifficultyLevel level = DifficultyLevel::Medium;
    
    // Terrain distribution based on difficulty
    TerrainDistribution terrainDist;
    
    // Maze complexity settings
    float deadEndRatio = 0.3f;      // Ratio of dead ends (0.0-1.0)
    float straightPathBias = 0.5f;  // Preference for straight paths (0.0-1.0)
    
    // Get difficulty name
    std::string getName() const {
        switch (level) {
            case DifficultyLevel::VeryEasy: return "Very Easy";
            case DifficultyLevel::Easy: return "Easy";
            case DifficultyLevel::Medium: return "Medium";
            case DifficultyLevel::Hard: return "Hard";
            case DifficultyLevel::VeryHard: return "Very Hard";
        }
        return "Unknown";
    }
    
    // Get star rating string
    std::string getStars() const {
        int stars = static_cast<int>(level);
        std::string result;
        for (int i = 0; i < 5; ++i) {
            result += (i < stars) ? "★" : "☆";
        }
        return result;
    }
    
    // Create settings from difficulty level
    static DifficultySettings fromLevel(DifficultyLevel level) {
        DifficultySettings settings;
        settings.level = level;
        
        switch (level) {
            case DifficultyLevel::VeryEasy:
                // Mostly normal terrain, few hazards
                settings.terrainDist = TerrainDistribution{
                    0.70f,  // normal
                    0.15f,  // grass
                    0.10f,  // sand
                    0.05f,  // water
                    0.00f,  // mud
                    0.00f,  // ice
                    0.00f   // lava
                };
                settings.terrainDist.clusterChance = 0.2f;
                settings.terrainDist.maxClusterSize = 5;
                settings.deadEndRatio = 0.2f;
                settings.straightPathBias = 0.7f;
                break;
                
            case DifficultyLevel::Easy:
                settings.terrainDist = TerrainDistribution{
                    0.55f,  // normal
                    0.20f,  // grass
                    0.15f,  // sand
                    0.07f,  // water
                    0.03f,  // mud
                    0.00f,  // ice
                    0.00f   // lava
                };
                settings.terrainDist.clusterChance = 0.25f;
                settings.terrainDist.maxClusterSize = 7;
                settings.deadEndRatio = 0.25f;
                settings.straightPathBias = 0.6f;
                break;
                
            case DifficultyLevel::Medium:
                settings.terrainDist = TerrainDistribution::balanced();
                settings.deadEndRatio = 0.3f;
                settings.straightPathBias = 0.5f;
                break;
                
            case DifficultyLevel::Hard:
                settings.terrainDist = TerrainDistribution{
                    0.30f,  // normal
                    0.15f,  // grass
                    0.15f,  // sand
                    0.15f,  // water
                    0.15f,  // mud
                    0.05f,  // ice
                    0.05f   // lava
                };
                settings.terrainDist.clusterChance = 0.4f;
                settings.terrainDist.maxClusterSize = 15;
                settings.deadEndRatio = 0.4f;
                settings.straightPathBias = 0.3f;
                break;
                
            case DifficultyLevel::VeryHard:
                settings.terrainDist = TerrainDistribution{
                    0.15f,  // normal
                    0.10f,  // grass
                    0.15f,  // sand
                    0.20f,  // water
                    0.20f,  // mud
                    0.10f,  // ice
                    0.10f   // lava
                };
                settings.terrainDist.clusterChance = 0.5f;
                settings.terrainDist.maxClusterSize = 20;
                settings.deadEndRatio = 0.5f;
                settings.straightPathBias = 0.2f;
                break;
        }
        
        return settings;
    }
    
    // Create from integer (1-5)
    static DifficultySettings fromInt(int level) {
        level = std::max(1, std::min(5, level));
        return fromLevel(static_cast<DifficultyLevel>(level));
    }
};

/**
 * Terrain distribution preset names and settings.
 */
struct TerrainPreset {
    const char* name;
    TerrainDistribution distribution;
    
    static const std::array<TerrainPreset, 6>& getPresets() {
        static const std::array<TerrainPreset, 6> presets = {{
            {"Balanced", TerrainDistribution::balanced()},
            {"Watery World", TerrainDistribution::watery()},
            {"Muddy Swamp", TerrainDistribution::swampy()},
            {"Volcanic", TerrainDistribution::volcanic()},
            {"Winter", TerrainDistribution::winter()},
            {"Custom", TerrainDistribution{}}  // User-defined
        }};
        return presets;
    }
};

} // namespace maze
