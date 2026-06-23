// ============================================================================
// NEON REQUIEM — NPC (Implementation)
// ============================================================================

#include "npc.h"
#include "../../core/display/display_manager.h"

NPC::NPC() : m_moveTimer(0), m_moveDir(0) {
    memset(&m_data, 0, sizeof(NPCData));
}

void NPC::begin(const NPCData& data) {
    m_data = data;
}

void NPC::update() {
    if (m_data.movePattern == 1) {
        m_moveTimer++;
        if (m_moveTimer > 60) {
            m_moveTimer = 0;
            m_moveDir = (m_moveDir + 1) % 4;
        }
    }
}

void NPC::render(int32_t cameraX, int32_t cameraY) {
    DisplayManager& display = DisplayManager::getInstance();
    int32_t sx = m_data.tileX * TILE_SIZE - cameraX;
    int32_t sy = m_data.tileY * TILE_SIZE - cameraY;

    if (sx < -TILE_SIZE || sx > DISPLAY_WIDTH || sy < -TILE_SIZE || sy > DISPLAY_HEIGHT) return;

    display.fillRect(sx, sy, TILE_SIZE, TILE_SIZE, m_data.color);
    display.drawRect(sx, sy, TILE_SIZE, TILE_SIZE, Palette::WHITE);
}

bool NPC::isInRange(float playerX, float playerY, int range) const {
    float dx = (m_data.tileX * TILE_SIZE + TILE_SIZE/2) - playerX;
    float dy = (m_data.tileY * TILE_SIZE + TILE_SIZE/2) - playerY;
    return (dx * dx + dy * dy) <= (range * range);
}