#include "UIPanel.hpp"
#include <sstream>
#include <iomanip>

namespace maze {

UIPanel::UIPanel() = default;

bool UIPanel::init(sf::RenderWindow& window) {
    // Try to load font from various locations
    std::vector<std::string> fontPaths = {
        // macOS system fonts
        "/System/Library/Fonts/SFNSMono.ttf",
        "/System/Library/Fonts/Menlo.ttc",
        "/System/Library/Fonts/Monaco.ttf",
        "/Library/Fonts/Arial.ttf",
        // Linux common fonts
        "/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf",
        "/usr/share/fonts/TTF/DejaVuSansMono.ttf",
        // Windows fonts
        "C:/Windows/Fonts/consola.ttf",
        "C:/Windows/Fonts/arial.ttf"
    };
    
    m_font.emplace();
    for (const auto& path : fontPaths) {
        if (m_font->openFromFile(path)) {
            m_fontLoaded = true;
            return true;
        }
    }
    
    m_font.reset();
    m_fontLoaded = false;
    return false;
}

void UIPanel::render(sf::RenderWindow& window) {
    if (!m_visible || !m_fontLoaded) return;
    
    // Save current view and switch to default view for UI rendering
    sf::View currentView = window.getView();
    window.setView(window.getDefaultView());
    
    // Render panels
    renderStatusPanel(window);
    renderStatsPanel(window);
    
    if (m_showHelp) {
        renderHelpPanel(window);
    }
    
    // Restore camera view
    window.setView(currentView);
}

void UIPanel::setGridSize(int width, int height) {
    m_gridWidth = width;
    m_gridHeight = height;
}

void UIPanel::setState(const std::string& state) {
    m_state = state;
}

void UIPanel::setAnimationEnabled(bool enabled) {
    m_animationEnabled = enabled;
}

void UIPanel::setAnimationDelay(int delayMs) {
    m_animationDelay = delayMs;
}

void UIPanel::setFPS(float fps) {
    m_fps = fps;
}

void UIPanel::setAlgorithmName(const std::string& name) {
    m_algorithmName = name;
}

void UIPanel::setNodesVisited(size_t count) {
    m_nodesVisited = count;
}

void UIPanel::setQueueSize(size_t size) {
    m_queueSize = size;
}

void UIPanel::setPathLength(size_t length) {
    m_pathLength = length;
}

void UIPanel::setElapsedTime(float timeMs) {
    m_elapsedTime = timeMs;
}

void UIPanel::toggleHelp() {
    m_showHelp = !m_showHelp;
}

void UIPanel::renderStatusPanel(sf::RenderWindow& window) {
    float panelWidth = 180.0f;
    float panelHeight = 130.0f;
    float x = m_padding;
    float y = m_padding;
    
    // Background
    renderPanel(window, x, y, panelWidth, panelHeight);
    
    // Content
    float textX = x + m_padding;
    float textY = y + m_padding;
    
    // Title
    renderText(window, "STATUS", textX, textY, m_headerColor);
    textY += m_lineHeight + 5;
    
    // Grid size
    std::stringstream ss;
    ss << m_gridWidth << " x " << m_gridHeight;
    renderTextPair(window, "Grid:", ss.str(), textX, textY);
    textY += m_lineHeight;
    
    // State
    renderTextPair(window, "State:", m_state, textX, textY);
    textY += m_lineHeight;
    
    // Animation
    renderTextPair(window, "Anim:", m_animationEnabled ? "ON" : "OFF", textX, textY);
    textY += m_lineHeight;
    
    // Delay
    ss.str("");
    ss << m_animationDelay << " ms";
    renderTextPair(window, "Delay:", ss.str(), textX, textY);
    textY += m_lineHeight;
    
    // FPS
    ss.str("");
    ss << std::fixed << std::setprecision(0) << m_fps;
    renderTextPair(window, "FPS:", ss.str(), textX, textY);
}

void UIPanel::renderStatsPanel(sf::RenderWindow& window) {
    if (m_algorithmName.empty()) return;
    
    float panelWidth = 180.0f;
    float panelHeight = 130.0f;
    float x = m_padding;
    float y = m_padding + 140.0f;
    
    // Background
    renderPanel(window, x, y, panelWidth, panelHeight);
    
    // Content
    float textX = x + m_padding;
    float textY = y + m_padding;
    
    // Title (algorithm name)
    std::string shortName = m_algorithmName;
    if (shortName.length() > 15) {
        shortName = shortName.substr(0, 3); // Just "BFS" or "DFS"
    }
    renderText(window, shortName, textX, textY, m_headerColor);
    textY += m_lineHeight + 5;
    
    // Visited
    std::stringstream ss;
    ss << m_nodesVisited;
    renderTextPair(window, "Visited:", ss.str(), textX, textY);
    textY += m_lineHeight;
    
    // Queue/Stack size
    ss.str("");
    ss << m_queueSize;
    renderTextPair(window, "Queue:", ss.str(), textX, textY);
    textY += m_lineHeight;
    
    // Path length
    ss.str("");
    ss << m_pathLength;
    renderTextPair(window, "Path:", ss.str(), textX, textY);
    textY += m_lineHeight;
    
    // Time
    ss.str("");
    ss << std::fixed << std::setprecision(1) << m_elapsedTime << " ms";
    renderTextPair(window, "Time:", ss.str(), textX, textY);
}

void UIPanel::renderHelpPanel(sf::RenderWindow& window) {
    sf::Vector2u windowSize = window.getSize();
    
    float panelWidth = 220.0f;
    float panelHeight = 340.0f;
    float x = windowSize.x - panelWidth - m_padding;
    float y = m_padding;
    
    // Background
    renderPanel(window, x, y, panelWidth, panelHeight);
    
    // Content
    float textX = x + m_padding;
    float textY = y + m_padding;
    
    // Title
    renderText(window, "CONTROLS", textX, textY, m_headerColor);
    textY += m_lineHeight + 8;
    
    // Control list
    std::vector<std::pair<std::string, std::string>> controls = {
        {"G", "Generate (animated)"},
        {"Enter", "Generate (instant)"},
        {"B", "Solve with BFS"},
        {"D", "Solve with DFS"},
        {"Space", "Toggle animation"},
        {"R", "Reset solution"},
        {"C", "Clear all"},
        {"+/-", "Adjust speed"},
        {"WASD", "Pan camera"},
        {"Scroll", "Zoom in/out"},
        {"F", "Fit to window"},
        {"1/2/3", "Grid size preset"},
        {"H", "Toggle this help"},
        {"ESC", "Exit"}
    };
    
    for (const auto& [key, desc] : controls) {
        // Key in highlight color
        renderText(window, key, textX, textY, m_highlightColor);
        // Description in white
        renderText(window, desc, textX + 60.0f, textY, m_textColor);
        textY += m_lineHeight;
    }
}

void UIPanel::renderPanel(sf::RenderWindow& window, float x, float y, float width, float height) {
    sf::RectangleShape panel({width, height});
    panel.setPosition({x, y});
    panel.setFillColor(m_bgColor);
    panel.setOutlineColor(sf::Color(60, 60, 60));
    panel.setOutlineThickness(1.0f);
    window.draw(panel);
}

void UIPanel::renderText(sf::RenderWindow& window, const std::string& text, float x, float y,
                         const sf::Color& color) {
    if (!m_fontLoaded) return;
    
    sf::Text sfText(*m_font, text, m_fontSize);
    sfText.setPosition({x, y});
    sfText.setFillColor(color);
    window.draw(sfText);
}

void UIPanel::renderTextPair(sf::RenderWindow& window, const std::string& label,
                             const std::string& value, float x, float y) {
    renderText(window, label, x, y, m_textColor);
    renderText(window, value, x + 70.0f, y, m_highlightColor);
}

} // namespace maze
