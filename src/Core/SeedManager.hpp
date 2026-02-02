#pragma once

#include <random>
#include <string>
#include <chrono>
#include <sstream>
#include <iomanip>

namespace maze {

/**
 * Manages random seeds for reproducible maze generation.
 * Allows saving and loading seeds to recreate exact mazes.
 */
class SeedManager {
public:
    SeedManager() {
        generateNewSeed();
    }
    
    // Generate a new random seed based on current time
    void generateNewSeed() {
        auto now = std::chrono::high_resolution_clock::now();
        auto duration = now.time_since_epoch();
        m_seed = static_cast<unsigned int>(
            std::chrono::duration_cast<std::chrono::microseconds>(duration).count()
        );
    }
    
    // Get current seed
    unsigned int getSeed() const { return m_seed; }
    
    // Set seed manually
    void setSeed(unsigned int seed) { m_seed = seed; }
    
    // Parse seed from string (supports hex format)
    bool parseSeed(const std::string& str) {
        try {
            // Check for hex format (0x prefix)
            if (str.length() > 2 && str[0] == '0' && (str[1] == 'x' || str[1] == 'X')) {
                m_seed = static_cast<unsigned int>(std::stoul(str, nullptr, 16));
            } else {
                m_seed = static_cast<unsigned int>(std::stoul(str));
            }
            return true;
        } catch (...) {
            return false;
        }
    }
    
    // Get seed as string (decimal)
    std::string getSeedString() const {
        return std::to_string(m_seed);
    }
    
    // Get seed as hex string
    std::string getSeedHex() const {
        std::ostringstream ss;
        ss << "0x" << std::hex << std::uppercase << std::setfill('0') 
           << std::setw(8) << m_seed;
        return ss.str();
    }
    
    // Get a display-friendly seed (shorter format)
    std::string getSeedDisplay() const {
        // Use last 6 digits for display
        return std::to_string(m_seed % 1000000);
    }

private:
    unsigned int m_seed = 0;
};

} // namespace maze
