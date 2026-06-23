// ============================================================================
// NEON REQUIEM — Cutscene Player (Implementation)
// ============================================================================

#include "cutscene_player.h"
#include "../../core/display/display_manager.h"

CutscenePlayer::CutscenePlayer()
    : m_spriteCount(0), m_bgColor(0x0000), m_parallax(0.0f), m_scrollX(0) {}

void CutscenePlayer::begin() { clearSprites(); }

void CutscenePlayer::update() { m_scrollX++; }

void CutscenePlayer::render() {
    DisplayManager& display = DisplayManager::getInstance();
    display.clearBuffer(m_bgColor);

    for (int i = 0; i < m_spriteCount; i++) {
        int sx = m_sprites[i].x - (m_scrollX * m_parallax);
        display.fillRect(sx, m_sprites[i].y, 16, 16, Palette::WHITE);
    }
}

void CutscenePlayer::setBackground(uint16_t color) { m_bgColor = color; }

void CutscenePlayer::addSprite(int16_t x, int16_t y, uint8_t spriteId, uint8_t frame) {
    if (m_spriteCount >= 16) return;
    m_sprites[m_spriteCount] = {x, y, spriteId, frame, 255};
    m_spriteCount++;
}

void CutscenePlayer::clearSprites() { m_spriteCount = 0; }

void CutscenePlayer::setParallax(float factor) { m_parallax = factor; }