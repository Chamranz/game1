/*
 * NEON REQUIEM - HUD Implementation
 * Renders player HP, RESONANCE energy, quest info, and minimap
 */

#include "hud.h"
#include "../../core/display/display_manager.h"
#include "config/palette.h"

// Singleton instance
HUD& HUD::getInstance() {
    static HUD instance;
    return instance;
}

void HUD::begin() {
    reset();
    LOG_INFO("HUD: Initialized");
}

void HUD::reset() {
    m_visible = true;
    m_visibilityFlags = HUD_SHOW_ALL;
    m_hpCurrent = 20;
    m_hpMax = 20;
    m_hpDisplay = 20;
    m_energyCurrent = 100;
    m_energyMax = 100;
    m_energyDisplay = 100;
    m_currentRoute = GameRoute::RESONANCE;
    m_hpFlashTimer = 0;
    m_questTitle[0] = '\0';
    m_questProgress = 0;
    m_questTotal = 0;
    m_locationName[0] = '\0';
    m_minimapWidth = 0;
    m_minimapHeight = 0;
}

void HUD::setHP(uint8_t current, uint8_t max) {
    m_hpCurrent = min(current, max);
    m_hpMax = max;
}

void HUD::setEnergy(uint8_t current, uint8_t max) {
    m_energyCurrent = min(current, max);
    m_energyMax = max;
}

void HUD::setQuestInfo(const char* questTitle, uint8_t progress, uint8_t total) {
    strncpy(m_questTitle, questTitle, sizeof(m_questTitle) - 1);
    m_questTitle[sizeof(m_questTitle) - 1] = '\0';
    m_questProgress = progress;
    m_questTotal = total;
}

void HUD::setRoute(GameRoute route) {
    m_currentRoute = route;
}

void HUD::setLocationName(const char* name) {
    strncpy(m_locationName, name, sizeof(m_locationName) - 1);
    m_locationName[sizeof(m_locationName) - 1] = '\0';
}

void HUD::setMinimapData(const uint8_t* data, uint8_t width, uint8_t height) {
    m_minimapWidth = min(width, (uint8_t)8);
    m_minimapHeight = min(height, (uint8_t)8);
    memcpy(m_minimapData, data, min((uint16_t)(width * height), (uint16_t)64));
}

void HUD::setPlayerMinimapPos(uint8_t x, uint8_t y) {
    m_playerMinimapX = x;
    m_playerMinimapY = y;
}

// ===== Update =====

void HUD::update() {
    // Animate HP bar toward actual value
    if (m_hpDisplay > m_hpCurrent) {
        m_hpDisplay = std::max<uint8_t>(m_hpCurrent, m_hpDisplay - 1);
        m_hpFlashTimer = 15; // Flash for 15 frames
    } else if (m_hpDisplay < m_hpCurrent) {
        m_hpDisplay = std::min<uint8_t>(m_hpCurrent, m_hpDisplay + 1);
    }
    
    // Animate energy bar
    if (m_energyDisplay > m_energyCurrent) {
        m_energyDisplay = std::max<uint8_t>(m_energyCurrent, m_energyDisplay - 2);
    } else if (m_energyDisplay < m_energyCurrent) {
        m_energyDisplay = std::min<uint8_t>(m_energyCurrent, m_energyDisplay + 2);
    }
    
    if (m_hpFlashTimer > 0) m_hpFlashTimer--;
}

// ===== Render =====

void HUD::render() {
    if (!m_visible) return;
    
    if (m_visibilityFlags & HUD_SHOW_HP) renderHPBar();
    if (m_visibilityFlags & HUD_SHOW_ENERGY) renderEnergyBar();
    if (m_visibilityFlags & HUD_SHOW_QUEST) renderQuestInfo();
    if (m_visibilityFlags & HUD_SHOW_MINIMAP) renderMinimap();
}

void HUD::renderHPBar() {
    DisplayManager& display = DisplayManager::getInstance();
    
    // HP bar background
    display.fillRect(HUD_HP_BAR_X, HUD_HP_BAR_Y, HUD_HP_BAR_W, HUD_HP_BAR_H, Palette::CONCRETE);
    
    // HP bar fill
    uint8_t hpRatio = (m_hpMax > 0) ? (m_hpDisplay * HUD_HP_BAR_W / m_hpMax) : 0;
    uint16_t hpColor;
    
    if (m_hpFlashTimer > 0 && (m_hpFlashTimer / 3) % 2 == 0) {
        hpColor = Palette::WHITE; // Flash white on damage
    } else if (m_hpDisplay > m_hpMax * 0.5) {
        hpColor = Palette::HP_FULL;
    } else if (m_hpDisplay > m_hpMax * 0.25) {
        hpColor = Palette::HP_MID;
    } else {
        hpColor = Palette::HP_LOW;
    }
    
    if (hpRatio > 0) {
        display.fillRect(HUD_HP_BAR_X, HUD_HP_BAR_Y, hpRatio, HUD_HP_BAR_H, hpColor);
    }
    
    // HP bar border
    display.drawRect(HUD_HP_BAR_X, HUD_HP_BAR_Y, HUD_HP_BAR_W, HUD_HP_BAR_H, Palette::WHITE);
    
    // HP text placeholder (no font system yet)
    // TODO: draw text "HP %d/%d" when font system is available
    display.fillRect(HUD_HP_BAR_X + 2, HUD_HP_BAR_Y, 40, 6, Palette::TEXT_WHITE);
}

void HUD::renderEnergyBar() {
    DisplayManager& display = DisplayManager::getInstance();
    
    // Energy bar background
    display.fillRect(HUD_ENERGY_BAR_X, HUD_ENERGY_BAR_Y, HUD_ENERGY_BAR_W, HUD_ENERGY_BAR_H, Palette::CONCRETE);
    
    // Energy bar fill
    uint8_t energyRatio = (m_energyMax > 0) ? (m_energyDisplay * HUD_ENERGY_BAR_W / m_energyMax) : 0;
    uint16_t energyColor;
    
    switch (m_currentRoute) {
        case GameRoute::RESONANCE: energyColor = Palette::CYAN; break;
        case GameRoute::ERASE:     energyColor = Palette::RED; break;
        case GameRoute::GLITCH:    energyColor = Palette::MAGENTA; break;
        default:              energyColor = Palette::CYAN; break;
    }
    
    if (energyRatio > 0) {
        display.fillRect(HUD_ENERGY_BAR_X, HUD_ENERGY_BAR_Y, energyRatio, HUD_ENERGY_BAR_H, energyColor);
    }
    
    display.drawRect(HUD_ENERGY_BAR_X, HUD_ENERGY_BAR_Y, HUD_ENERGY_BAR_W, HUD_ENERGY_BAR_H, Palette::WHITE);
}

void HUD::renderQuestInfo() {
    if (m_questTitle[0] == '\0') return;
    
    DisplayManager& display = DisplayManager::getInstance();
    
    // Quest text at bottom of screen
    uint16_t questColor;
    switch (m_currentRoute) {
        case GameRoute::RESONANCE: questColor = Palette::CYAN; break;
        case GameRoute::ERASE:     questColor = Palette::RED; break;
        case GameRoute::GLITCH:    questColor = Palette::MAGENTA; break;
        default:              questColor = Palette::WHITE; break;
    }
    
    // TODO: drawText when font system is available
    display.fillRect(HUD_QUEST_X, HUD_QUEST_Y, strlen(m_questTitle) * 6, 8, questColor);
    
    // Progress indicator
    if (m_questTotal > 0) {
        char progressText[12];
        snprintf(progressText, sizeof(progressText), "%d/%d", m_questProgress, m_questTotal);
        // TODO: drawText when font system is available
        display.fillRect(HUD_QUEST_X + 100, HUD_QUEST_Y, 40, 8, Palette::TEXT_WHITE);
    }
}

void HUD::renderMinimap() {
    if (m_minimapWidth == 0 || m_minimapHeight == 0) return;
    
    DisplayManager& display = DisplayManager::getInstance();
    
    // Minimap background
    display.fillRect(HUD_MINIMAP_X, HUD_MINIMAP_Y, HUD_MINIMAP_SIZE, HUD_MINIMAP_SIZE, Palette::BLACK);
    display.drawRect(HUD_MINIMAP_X, HUD_MINIMAP_Y, HUD_MINIMAP_SIZE, HUD_MINIMAP_SIZE, Palette::CONCRETE);
    
    // Calculate tile size
    uint8_t tileSize = HUD_MINIMAP_SIZE / max(m_minimapWidth, m_minimapHeight);
    
    // Draw minimap tiles
    for (int y = 0; y < m_minimapHeight; y++) {
        for (int x = 0; x < m_minimapWidth; x++) {
            uint8_t tile = m_minimapData[y * m_minimapWidth + x];
            if (tile > 0) {
                uint16_t color;
                switch (tile) {
                    case 1:  color = Palette::CONCRETE; break; // Floor
                    case 2:  color = Palette::NIGHT_BLUE; break; // Wall
                    case 3:  color = Palette::GREEN; break;     // Grass
                    case 4:  color = Palette::BLUE; break;      // Water
                    default: color = Palette::CONCRETE; break;
                }
                display.fillRect(HUD_MINIMAP_X + x * tileSize, HUD_MINIMAP_Y + y * tileSize, 
                                 tileSize, tileSize, color);
            }
        }
    }
    
    // Draw player position
    uint8_t px = HUD_MINIMAP_X + m_playerMinimapX * tileSize + tileSize / 2 - 1;
    uint8_t py = HUD_MINIMAP_Y + m_playerMinimapY * tileSize + tileSize / 2 - 1;
    display.fillRect(px, py, 3, 3, Palette::WHITE);
}