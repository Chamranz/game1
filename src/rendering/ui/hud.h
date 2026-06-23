/*
 * NEON REQUIEM - HUD (Heads-Up Display)
 * Renders player HP, RESONANCE energy, quest info, and minimap
 * Route-specific color theming
 */

#ifndef HUD_H
#define HUD_H

#include <Arduino.h>
#include "config/config.h"

// HUD element positions
#define HUD_HP_BAR_X 4
#define HUD_HP_BAR_Y 4
#define HUD_HP_BAR_W 80
#define HUD_HP_BAR_H 6
#define HUD_ENERGY_BAR_X 4
#define HUD_ENERGY_BAR_Y 12
#define HUD_ENERGY_BAR_W 60
#define HUD_ENERGY_BAR_H 4
#define HUD_QUEST_X 4
#define HUD_QUEST_Y 224
#define HUD_MINIMAP_SIZE 40
#define HUD_MINIMAP_X (DISPLAY_WIDTH - HUD_MINIMAP_SIZE - 4)
#define HUD_MINIMAP_Y 4

// HUD visibility flags
#define HUD_SHOW_HP       0x01
#define HUD_SHOW_ENERGY   0x02
#define HUD_SHOW_QUEST    0x04
#define HUD_SHOW_MINIMAP  0x08
#define HUD_SHOW_ALL      0xFF

class HUD {
public:
    // Singleton access
    static HUD& getInstance();
    
    // Control
    void begin();
    void reset();
    void setVisible(bool visible) { m_visible = visible; }
    bool isVisible() const { return m_visible; }
    void setVisibilityFlags(uint8_t flags) { m_visibilityFlags = flags; }
    uint8_t getVisibilityFlags() const { return m_visibilityFlags; }
    
    // Data update
    void setHP(uint8_t current, uint8_t max);
    void setEnergy(uint8_t current, uint8_t max);
    void setQuestInfo(const char* questTitle, uint8_t progress, uint8_t total);
    void setRoute(GameRoute route);
    void setLocationName(const char* name);
    
    // Minimap
    void setMinimapData(const uint8_t* data, uint8_t width, uint8_t height);
    void setPlayerMinimapPos(uint8_t x, uint8_t y);
    
    // Render
    void render();
    
    // Animation
    void update();  // Update animations (HP flash, etc.)

private:
    HUD() : m_visible(true), m_visibilityFlags(HUD_SHOW_ALL),
            m_hpCurrent(20), m_hpMax(20), m_hpDisplay(20),
            m_energyCurrent(100), m_energyMax(100), m_energyDisplay(100),
            m_currentRoute(GameRoute::RESONANCE), m_hpFlashTimer(0) {}
    ~HUD() {}
    HUD(const HUD&) = delete;
    HUD& operator=(const HUD&) = delete;
    
    void renderHPBar();
    void renderEnergyBar();
    void renderQuestInfo();
    void renderMinimap();
    
    bool m_visible;
    uint8_t m_visibilityFlags;
    
    // HP
    uint8_t m_hpCurrent;
    uint8_t m_hpMax;
    uint8_t m_hpDisplay;  // Animated display value
    
    // Energy
    uint8_t m_energyCurrent;
    uint8_t m_energyMax;
    uint8_t m_energyDisplay;  // Animated display value
    
    // Quest
    char m_questTitle[32];
    uint8_t m_questProgress;
    uint8_t m_questTotal;
    
    // Location
    char m_locationName[24];
    
    // Route
    GameRoute m_currentRoute;
    
    // Minimap
    uint8_t m_minimapData[64];
    uint8_t m_minimapWidth;
    uint8_t m_minimapHeight;
    uint8_t m_playerMinimapX;
    uint8_t m_playerMinimapY;
    
    // Animation
    uint8_t m_hpFlashTimer;
};

#endif // HUD_H