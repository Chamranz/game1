/*
 * NEON REQUIEM - Menu UI
 * Renders game menus: pause, inventory, save, settings
 * Route-specific styling with cyberpunk aesthetic
 */

#ifndef MENU_UI_H
#define MENU_UI_H

#include <Arduino.h>
#include "config/config.h"

// Menu types
enum MenuType : uint8_t {
    MENU_PAUSE = 0,
    MENU_INVENTORY = 1,
    MENU_EQUIP = 2,
    MENU_SAVE = 3,
    MENU_SETTINGS = 4,
    MENU_TITLE = 5,
    MENU_ROUTE_SELECT = 6
};

// Menu item
struct MenuItem {
    const char* label;
    void (*callback)();
    bool enabled;
    uint8_t iconIndex;
};

// Menu state
struct MenuState {
    MenuType type;
    const char* title;
    uint8_t itemCount;
    uint8_t selectedIndex;
    uint8_t scrollOffset;
    uint16_t backgroundColor;
    uint16_t highlightColor;
    uint16_t textColor;
};

class MenuUI {
public:
    // Singleton access
    static MenuUI& getInstance();
    
    // Control
    void begin();
    void reset();
    
    // Menu management
    void openMenu(MenuType type, const char* title);
    void closeMenu();
    bool isOpen() const { return m_open; }
    MenuType getCurrentMenu() const { return m_state.type; }
    
    // Navigation
    void navigateUp();
    void navigateDown();
    void navigateLeft();
    void navigateRight();
    uint8_t getSelectedIndex() const { return m_state.selectedIndex; }
    
    // Menu items
    void setItems(const MenuItem* items, uint8_t count);
    void setItemEnabled(uint8_t index, bool enabled);
    
    // Rendering
    void render();
    void renderTitle();
    void renderMenuItems();
    void renderScrollIndicator();
    void renderBackground();
    
    // Sub-menu rendering
    void renderInventoryMenu();
    void renderSaveMenu();
    void renderSettingsMenu();
    
    // Configuration
    void setHighlightColor(uint16_t color) { m_state.highlightColor = color; }
    void setBackgroundColor(uint16_t color) { m_state.backgroundColor = color; }

private:
    MenuUI() : m_open(false), m_itemCount(0) {}
    ~MenuUI() {}
    MenuUI(const MenuUI&) = delete;
    MenuUI& operator=(const MenuUI&) = delete;
    
    bool m_open;
    MenuState m_state;
    MenuItem m_items[16];
    uint8_t m_itemCount;
};

#endif // MENU_UI_H