// ============================================================================
// NEON REQUIEM — Cutscene Player (Header)
// ============================================================================
// Проигрыватель кадров катсцены с поддержкой слоёв, параллакса и эффектов.
// ============================================================================

#ifndef NEON_REQUIEM_CUTSCENE_PLAYER_H
#define NEON_REQUIEM_CUTSCENE_PLAYER_H

#include <cstdint>
#include <Arduino.h>
#include "config/config.h"

struct CutsceneSprite {
    int16_t x, y;
    uint8_t spriteId;
    uint8_t frame;
    uint8_t alpha;
};

class CutscenePlayer {
public:
    CutscenePlayer();

    void begin();
    void update();
    void render();

    void setBackground(uint16_t color);
    void addSprite(int16_t x, int16_t y, uint8_t spriteId, uint8_t frame);
    void clearSprites();
    void setParallax(float factor);

private:
    CutsceneSprite m_sprites[16];
    int m_spriteCount;
    uint16_t m_bgColor;
    float m_parallax;
    int m_scrollX;
};

#endif