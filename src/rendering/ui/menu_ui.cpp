/*
 * NEON REQUIEM - Menu UI Implementation
 * Renders game menus: pause, inventory, save, settings
 */

#include "menu_ui.h"
#include "../../core/display/display_manager.h"
#include "config/palette.h"

// Singleton instance
MenuUI& MenuUI::getInstance() {
    static MenuUI instance;
    return instance;
}

void MenuUI::begin() {
    reset();
    LOG_INFO("MenuUI: Initialized");
}

void MenuUI::reset() {
    m_open = false;
    m_itemCount = 0;
    m_state.type = MENU_PAUSE;
    m_state.title = "";
    m_state.itemCount = 0;
    m_state.selectedIndex = 0;
    m_state.scrollOffset = 0;
    m_state.backgroundColor = Palette::BLACK;
    m_state.highlightColor = Palette::CYAN;
    m_state.textColor = Palette::WHITE;
}

void MenuUI::openMenu(MenuType type, const char* title) {
    m_open = true;
    m_state.type = type;
    m_state.title = title;
    m_state.selectedIndex = 0;
    m_state.scrollOffset = 0;
    
    // Set colors based on menu type
    switch (type) {
        case MENU_PAUSE:
            m_state.backgroundColor = Palette::BLACK;
            m_state.highlightColor = Palette::CYAN;
            break;
        case MENU_INVENTORY:
            m_state.backgroundColor = Palette::NIGHT_BLUE;
            m_state.highlightColor = Palette::AMBER;
            break;
        case MENU_SAVE:
            m_state.backgroundColor = Palette::BLACK;
            m_state.highlightColor = Palette::GREEN;
            break;
        case MENU_SETTINGS:
            m_state.backgroundColor = Palette::CONCRETE;
            m_state.highlightColor = Palette::WHITE;
            break;
        case MENU_TITLE:
            m_state.backgroundColor = Palette::BLACK;
            m_state.highlightColor = Palette::CYAN;
            break;
        default:
            break;
    }
}

void MenuUI::closeMenu() {
    m_open = false;
}

void MenuUI::navigateUp() {
    if (m_state.selectedIndex > 0) {
        m_state.selectedIndex--;
        if (m_state.selectedIndex < m_state.scrollOffset) {
            m_state.scrollOffset = m_state.selectedIndex;
        }
    }
}

void MenuUI::navigateDown() {
    if (m_state.selectedIndex < m_state.itemCount - 1) {
        m_state.selectedIndex++;
        uint8_t maxVisible = 8;
        if (m_state.selectedIndex >= m_state.scrollOffset + maxVisible) {
            m_state.scrollOffset = m_state.selectedIndex - maxVisible + 1;
        }
    }
}

void MenuUI::navigateLeft() {
    // Sub-menu navigation (e.g., settings sliders)
}

void MenuUI::navigateRight() {
    // Sub-menu navigation (e.g., settings sliders)
}

void MenuUI::setItems(const MenuItem* items, uint8_t count) {
    m_itemCount = min(count, (uint8_t)16);
    for (int i = 0; i < m_itemCount; i++) {
        m_items[i] = items[i];
    }
    m_state.itemCount = m_itemCount;
}

void MenuUI::setItemEnabled(uint8_t index, bool enabled) {
    if (index < m_itemCount) {
        m_items[index].enabled = enabled;
    }
}

// ===== Rendering =====

void MenuUI::render() {
    if (!m_open) return;
    
    renderBackground();
    renderTitle();
    
    switch (m_state.type) {
        case MENU_INVENTORY:
            renderInventoryMenu();
            break;
        case MENU_SAVE:
            renderSaveMenu();
            break;
        case MENU_SETTINGS:
            renderSettingsMenu();
            break;
        default:
            renderMenuItems();
            break;
    }
    
    renderScrollIndicator();
}

void MenuUI::renderBackground() {
    DisplayManager& display = DisplayManager::getInstance();
    
    // Semi-transparent overlay
    display.fillRect(0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, m_state.backgroundColor);
    
    // Decorative border
    display.drawRect(2, 2, DISPLAY_WIDTH - 4, DISPLAY_HEIGHT - 4, m_state.highlightColor);
}

void MenuUI::renderTitle() {
    DisplayManager& display = DisplayManager::getInstance();
    
    // TODO: drawText when font system is available
    // Draw menu title placeholder
    display.fillRect(DISPLAY_WIDTH / 2 - strlen(m_state.title) * 3, 8, 
                     strlen(m_state.title) * 6, 8, m_state.highlightColor);
    
    // Draw separator line
    display.drawLine(4, 20, DISPLAY_WIDTH - 8, 20, m_state.highlightColor);
}

void MenuUI::renderMenuItems() {
    DisplayManager& display = DisplayManager::getInstance();
    
    uint8_t startY = 28;
    uint8_t maxVisible = 8;
    
    for (int i = m_state.scrollOffset;
         i < std::min<int>(m_state.itemCount, m_state.scrollOffset + maxVisible); i++) {
        uint8_t y = startY + (i - m_state.scrollOffset) * 14;
        
        // Highlight selected item
        if (i == m_state.selectedIndex) {
            display.fillRect(8, y - 1, DISPLAY_WIDTH - 16, 12, m_state.highlightColor);
            // TODO: drawText when font system is available
            display.fillRect(12, y, strlen(m_items[i].label) * 6, 8, Palette::BLACK);
        } else {
            uint16_t color = m_items[i].enabled ? m_state.textColor : Palette::CONCRETE;
            // TODO: drawText when font system is available
            display.fillRect(12, y, strlen(m_items[i].label) * 6, 8, color);
        }
    }
}

void MenuUI::renderScrollIndicator() {
    DisplayManager& display = DisplayManager::getInstance();
    
    if (m_state.scrollOffset > 0) {
        // Up arrow placeholder
        // TODO: drawText when font system is available
        display.fillRect(DISPLAY_WIDTH / 2 - 3, 22, 6, 8, m_state.highlightColor);
    }
    
    uint8_t maxVisible = 8;
    if (m_state.scrollOffset + maxVisible < m_state.itemCount) {
        // Down arrow placeholder
        // TODO: drawText when font system is available
        display.fillRect(DISPLAY_WIDTH / 2 - 3, DISPLAY_HEIGHT - 10, 6, 8, m_state.highlightColor);
    }
}

void MenuUI::renderInventoryMenu() {
    DisplayManager& display = DisplayManager::getInstance();
    
    // TODO: drawText when font system is available
    display.fillRect(8, 28, 40, 8, Palette::WHITE);
    
    // Show item categories
    const char* categories[] = {"Consumables", "Key Items", "Data Fragments", "Equipment"};
    for (int i = 0; i < 4; i++) {
        uint8_t y = 42 + i * 14;
        uint16_t color = (i == m_state.selectedIndex) ? m_state.highlightColor : Palette::WHITE;
        // TODO: drawText when font system is available
        display.fillRect(12, y, strlen(categories[i]) * 6, 8, color);
    }
}

void MenuUI::renderSaveMenu() {
    DisplayManager& display = DisplayManager::getInstance();
    
    // Save slot rendering
    for (int i = 0; i < 3; i++) {
        uint8_t y = 28 + i * 30;
        uint16_t color = (i == m_state.selectedIndex) ? m_state.highlightColor : Palette::WHITE;
        
        // TODO: drawText when font system is available
        display.fillRect(12, y, 40, 8, color);
        display.fillRect(12, y + 10, 40, 8, Palette::CONCRETE);
    }
}

void MenuUI::renderSettingsMenu() {
    DisplayManager& display = DisplayManager::getInstance();
    
    // Settings items
    const char* settings[] = {"Music Volume", "SFX Volume", "Text Speed", "CRT Effect", "Brightness"};
    for (int i = 0; i < 5; i++) {
        uint8_t y = 28 + i * 14;
        uint16_t color = (i == m_state.selectedIndex) ? m_state.highlightColor : Palette::WHITE;
        // TODO: drawText when font system is available
        display.fillRect(12, y, strlen(settings[i]) * 6, 8, color);
    }
}