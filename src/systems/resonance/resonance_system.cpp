// ============================================================================
// NEON REQUIEM — Resonance System (Implementation)
// ============================================================================

#include "resonance_system.h"
#include "../../core/display/display_manager.h"

ResonanceSystem::ResonanceSystem()
    : m_objectCount(0), m_active(false), m_energyTimer(0), m_pulseTimer(0) {}

void ResonanceSystem::begin() { clearObjects(); }

void ResonanceSystem::update() {
    if (!m_active) return;
    m_pulseTimer++;
}

void ResonanceSystem::render(int32_t cameraX, int32_t cameraY) {
    if (!m_active) return;

    DisplayManager& display = DisplayManager::getInstance();
    float pulse = sin(m_pulseTimer * 0.1f) * 0.3f + 0.7f;

    for (int i = 0; i < m_objectCount; i++) {
        if (!m_objects[i].active) continue;

        int sx = m_objects[i].x - cameraX;
        int sy = m_objects[i].y - cameraY;
        uint16_t c = Palette::lerpColor(m_objects[i].color, Palette::WHITE, pulse * 0.3f);

        display.drawRect(sx, sy, m_objects[i].w, m_objects[i].h, c);
        display.fillRect(sx + 1, sy + 1, m_objects[i].w - 2, m_objects[i].h - 2,
                        Palette::darkenColor(c, 0.8f));
    }
}

void ResonanceSystem::activate() { m_active = true; m_pulseTimer = 0; }
void ResonanceSystem::deactivate() { m_active = false; }

void ResonanceSystem::addObject(int16_t x, int16_t y, int16_t w, int16_t h,
                                 uint16_t color, const char* info) {
    if (m_objectCount >= 16) return;
    ResonanceObject& obj = m_objects[m_objectCount];
    obj.x = x;
    obj.y = y;
    obj.w = w;
    obj.h = h;
    obj.color = color;
    obj.info[0] = '\0';
    obj.active = true;
    strncpy(obj.info, info, sizeof(obj.info) - 1);
    obj.info[sizeof(obj.info) - 1] = '\0';
    m_objectCount++;
}

void ResonanceSystem::clearObjects() { m_objectCount = 0; }