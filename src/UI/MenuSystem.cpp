#include "MenuSystem.hpp"
#include <sstream>

namespace maze {

MenuSystem::MenuSystem() {
    // Initialize size presets
    m_sizePresets = {
        {"Small (25x25)", 25, 25},
        {"Medium (50x50)", 50, 50},
        {"Large (100x100)", 100, 100},
        {"XLarge (200x200)", 200, 200},
        {"Huge (500x500)", 500, 500}
    };
}

bool MenuSystem::init(sf::RenderWindow& window) {
    // Load font
    std::vector<std::string> fontPaths = {
        "/System/Library/Fonts/SFNSMono.ttf",
        "/System/Library/Fonts/Menlo.ttc",
        "/System/Library/Fonts/Monaco.ttf",
        "/Library/Fonts/Arial.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf",
        "C:/Windows/Fonts/consola.ttf"
    };
    
    m_font.emplace();
    for (const auto& path : fontPaths) {
        if (m_font->openFromFile(path)) {
            m_fontLoaded = true;
            break;
        }
    }
    
    if (!m_fontLoaded) {
        m_font.reset();
        return false;
    }
    
    // Build default menu structure
    clearSections();
    
    // Generation Algorithm section
    MenuSection genSection;
    genSection.title = "GENERATION ALGORITHM";
    genSection.items = {
        {"Algorithm", ItemType::Selector, 
         {"Recursive Backtracking", "Prim's Algorithm", "Kruskal's Algorithm", 
          "Binary Tree", "Eller's Algorithm"}, 
         m_selectedGenerator, nullptr, 
         [this](int idx) { 
             m_selectedGenerator = idx; 
             if (onGeneratorChange) onGeneratorChange(idx);
         }}
    };
    addSection(genSection);
    
    // Solving Algorithm section
    MenuSection solveSection;
    solveSection.title = "SOLVING ALGORITHM";
    solveSection.items = {
        {"Algorithm", ItemType::Selector,
         {"BFS", "DFS", "A* (A-Star)", "Dijkstra", "Greedy Best-First", "Bidirectional BFS"},
         m_selectedSolver, nullptr,
         [this](int idx) {
             m_selectedSolver = idx;
             if (onSolverChange) onSolverChange(idx);
         }}
    };
    addSection(solveSection);
    
    // Grid Size section
    MenuSection sizeSection;
    sizeSection.title = "GRID SIZE";
    std::vector<std::string> sizeOptions;
    for (const auto& preset : m_sizePresets) {
        sizeOptions.push_back(preset.name);
    }
    sizeSection.items = {
        {"Size", ItemType::Selector, sizeOptions, m_currentSizePreset, nullptr,
         [this](int idx) {
             m_currentSizePreset = idx;
             m_gridWidth = m_sizePresets[idx].width;
             m_gridHeight = m_sizePresets[idx].height;
             if (onSizeChange) onSizeChange(m_gridWidth, m_gridHeight);
         }}
    };
    addSection(sizeSection);
    
    // Difficulty section
    MenuSection diffSection;
    diffSection.title = "DIFFICULTY";
    diffSection.items = {
        {"Level", ItemType::Selector,
         {"1 - Very Easy", "2 - Easy", "3 - Medium", "4 - Hard", "5 - Very Hard"},
         m_difficulty - 1, nullptr,
         [this](int idx) {
             m_difficulty = idx + 1;
             if (onDifficultyChange) onDifficultyChange(m_difficulty);
         }}
    };
    addSection(diffSection);
    
    // Actions section
    MenuSection actSection;
    actSection.title = "ACTIONS";
    actSection.items = {
        {"Apply & Close", ItemType::Action, {}, 0, 
         [this]() { 
             if (onApply) onApply();
             hide();
         }, nullptr},
        {"Cancel", ItemType::Action, {}, 0, [this]() { hide(); }, nullptr}
    };
    addSection(actSection);
    
    return true;
}

void MenuSystem::show() {
    m_visible = true;
    m_currentSection = 0;
    m_currentItem = 0;
}

void MenuSystem::hide() {
    m_visible = false;
}

void MenuSystem::toggle() {
    if (m_visible) hide();
    else show();
}

void MenuSystem::addSection(const MenuSection& section) {
    m_sections.push_back(section);
}

void MenuSystem::clearSections() {
    m_sections.clear();
    m_currentSection = 0;
    m_currentItem = 0;
}

bool MenuSystem::handleKeyPress(sf::Keyboard::Key key) {
    if (!m_visible) return false;
    
    switch (key) {
        case sf::Keyboard::Key::Up:
        case sf::Keyboard::Key::W:
            moveUp();
            return true;
            
        case sf::Keyboard::Key::Down:
        case sf::Keyboard::Key::S:
            moveDown();
            return true;
            
        case sf::Keyboard::Key::Left:
        case sf::Keyboard::Key::A:
            cycleOption(-1);
            return true;
            
        case sf::Keyboard::Key::Right:
        case sf::Keyboard::Key::D:
            cycleOption(1);
            return true;
            
        case sf::Keyboard::Key::Enter:
            selectItem();
            return true;
            
        case sf::Keyboard::Key::Escape:
        case sf::Keyboard::Key::M:
            hide();
            return true;
            
        default:
            return false;
    }
}

void MenuSystem::moveUp() {
    int flatIdx = getFlatIndex(m_currentSection, m_currentItem);
    if (flatIdx > 0) {
        flatIdx--;
        getItemPosition(flatIdx, m_currentSection, m_currentItem);
    }
}

void MenuSystem::moveDown() {
    int flatIdx = getFlatIndex(m_currentSection, m_currentItem);
    int total = getTotalItems();
    if (flatIdx < total - 1) {
        flatIdx++;
        getItemPosition(flatIdx, m_currentSection, m_currentItem);
    }
}

void MenuSystem::selectItem() {
    if (m_currentSection >= static_cast<int>(m_sections.size())) return;
    
    auto& section = m_sections[m_currentSection];
    if (m_currentItem >= static_cast<int>(section.items.size())) return;
    
    auto& item = section.items[m_currentItem];
    
    if (item.type == ItemType::Action && item.callback) {
        item.callback();
    } else if (item.type == ItemType::Selector) {
        cycleOption(1);
    }
}

void MenuSystem::cycleOption(int direction) {
    if (m_currentSection >= static_cast<int>(m_sections.size())) return;
    
    auto& section = m_sections[m_currentSection];
    if (m_currentItem >= static_cast<int>(section.items.size())) return;
    
    auto& item = section.items[m_currentItem];
    
    if (item.type == ItemType::Selector && !item.options.empty()) {
        int numOptions = static_cast<int>(item.options.size());
        item.selectedOption = (item.selectedOption + direction + numOptions) % numOptions;
        
        if (item.onOptionChange) {
            item.onOptionChange(item.selectedOption);
        }
    }
}

int MenuSystem::getTotalItems() const {
    int total = 0;
    for (const auto& section : m_sections) {
        total += static_cast<int>(section.items.size());
    }
    return total;
}

void MenuSystem::getItemPosition(int flatIndex, int& section, int& item) const {
    int count = 0;
    for (size_t s = 0; s < m_sections.size(); ++s) {
        int sectionSize = static_cast<int>(m_sections[s].items.size());
        if (count + sectionSize > flatIndex) {
            section = static_cast<int>(s);
            item = flatIndex - count;
            return;
        }
        count += sectionSize;
    }
    section = 0;
    item = 0;
}

int MenuSystem::getFlatIndex(int section, int item) const {
    int idx = 0;
    for (int s = 0; s < section && s < static_cast<int>(m_sections.size()); ++s) {
        idx += static_cast<int>(m_sections[s].items.size());
    }
    return idx + item;
}

void MenuSystem::render(sf::RenderWindow& window) {
    if (!m_visible || !m_fontLoaded) return;
    
    // Save current view and switch to default
    sf::View currentView = window.getView();
    window.setView(window.getDefaultView());
    
    // Calculate menu position (centered)
    sf::Vector2u windowSize = window.getSize();
    float menuX = (windowSize.x - m_menuWidth) / 2.0f;
    float menuY = (windowSize.y - m_menuHeight) / 2.0f;
    
    // Render background
    sf::RectangleShape bg({m_menuWidth, m_menuHeight});
    bg.setPosition({menuX, menuY});
    bg.setFillColor(m_bgColor);
    bg.setOutlineColor(m_borderColor);
    bg.setOutlineThickness(2.0f);
    window.draw(bg);
    
    // Render title
    float y = menuY + m_padding;
    renderText(window, "MAZE MENU", menuX + m_menuWidth / 2.0f - 50.0f, y, 
               m_titleColor, m_titleFontSize);
    y += m_titleFontSize + m_padding;
    
    // Render separator
    sf::RectangleShape separator({m_menuWidth - 2 * m_padding, 1.0f});
    separator.setPosition({menuX + m_padding, y});
    separator.setFillColor(m_separatorColor);
    window.draw(separator);
    y += m_padding;
    
    // Render sections
    for (size_t s = 0; s < m_sections.size(); ++s) {
        renderSection(window, m_sections[s], y, static_cast<int>(s));
    }
    
    // Render navigation hint
    y = menuY + m_menuHeight - m_padding - m_fontSize;
    renderText(window, "Arrow Keys: Navigate | Enter: Select | Esc: Close",
               menuX + m_padding, y, m_separatorColor, m_fontSize - 2);
    
    // Restore view
    window.setView(currentView);
}

void MenuSystem::renderSection(sf::RenderWindow& window, const MenuSection& section,
                               float& y, int sectionIdx) {
    sf::Vector2u windowSize = window.getSize();
    float menuX = (windowSize.x - m_menuWidth) / 2.0f;
    
    // Section title
    renderText(window, section.title, menuX + m_padding, y, m_titleColor, m_fontSize);
    y += m_fontSize + 5.0f;
    
    // Items
    for (size_t i = 0; i < section.items.size(); ++i) {
        const auto& item = section.items[i];
        bool isSelected = (sectionIdx == m_currentSection && 
                          static_cast<int>(i) == m_currentItem);
        
        sf::Color labelColor = isSelected ? m_selectedColor : m_textColor;
        
        // Selection indicator
        if (isSelected) {
            sf::RectangleShape selector({m_menuWidth - 2 * m_padding, m_itemHeight - 4});
            selector.setPosition({menuX + m_padding, y - 2});
            selector.setFillColor(sf::Color(40, 40, 40));
            window.draw(selector);
            
            renderText(window, ">", menuX + m_padding + 5, y, m_selectedColor);
        }
        
        // Item label
        float labelX = menuX + m_padding + (isSelected ? 25.0f : 10.0f);
        renderText(window, item.label + ":", labelX, y, labelColor);
        
        // Item value (for selectors)
        if (item.type == ItemType::Selector && !item.options.empty()) {
            std::string value = "< " + item.options[item.selectedOption] + " >";
            float valueX = menuX + m_menuWidth - m_padding - 180.0f;
            renderText(window, value, valueX, y, m_valueColor);
        }
        
        y += m_itemHeight;
    }
    
    y += m_sectionSpacing / 2.0f;
}

void MenuSystem::renderText(sf::RenderWindow& window, const std::string& text,
                            float x, float y, const sf::Color& color, unsigned int size) {
    if (!m_fontLoaded) return;
    
    unsigned int fontSize = (size > 0) ? size : m_fontSize;
    sf::Text sfText(*m_font, text, fontSize);
    sfText.setPosition({x, y});
    sfText.setFillColor(color);
    window.draw(sfText);
}

} // namespace maze
