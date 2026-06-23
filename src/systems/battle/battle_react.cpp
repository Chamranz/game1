// ============================================================================
// NEON REQUIEM — Battle Reaction System (Implementation)
// ============================================================================

#include "battle_react.h"
#include <cstdlib>

BattleReact::BattleReact() : m_timer(0), m_spawnTimer(0), m_finished(false) {
    m_pattern = {0, 8, 2.0f, 0xF800, 60};
}

void BattleReact::beginAttack(AttackPattern pattern, SoulBox& soulBox) {
    m_pattern = pattern;
    m_timer = 0;
    m_spawnTimer = 0;
    m_finished = false;
    soulBox.clearBullets();
}

void BattleReact::update(SoulBox& soulBox) {
    m_timer++;
    m_spawnTimer++;

    if (m_timer >= m_pattern.duration) {
        m_finished = true;
        return;
    }

    // Спавн пуль каждые N кадров
    int spawnRate = max(1, 10 - m_pattern.bulletCount / 2);
    if (m_spawnTimer >= spawnRate) {
        m_spawnTimer = 0;

        switch (m_pattern.type) {
            case 0: spawnSpiral(soulBox); break;
            case 1: spawnWave(soulBox); break;
            case 2: spawnRain(soulBox); break;
            case 3: spawnCross(soulBox); break;
        }
    }
}

void BattleReact::spawnSpiral(SoulBox& soulBox) {
    float cx = soulBox.getSoulX();
    float cy = soulBox.getSoulY();
    float angle = m_timer * 0.3f;

    for (int i = 0; i < 4; i++) {
        float a = angle + i * 3.14159f / 2;
        soulBox.addBullet(cx, cy, cos(a) * m_pattern.speed,
                          sin(a) * m_pattern.speed, m_pattern.color, 3);
    }
}

void BattleReact::spawnWave(SoulBox& soulBox) {
    float cx = soulBox.getSoulX();
    float cy = soulBox.getSoulY();
    float offset = sin(m_timer * 0.1f) * 30;

    for (int i = -2; i <= 2; i++) {
        soulBox.addBullet(cx + offset + i * 15, cy - 30, 0,
                          m_pattern.speed, m_pattern.color, 3);
    }
}

void BattleReact::spawnRain(SoulBox& soulBox) {
    float cx = soulBox.getSoulX();
    float cy = soulBox.getSoulY();

    for (int i = 0; i < 3; i++) {
        float rx = cx + (random(-20, 20));
        soulBox.addBullet(rx, cy - 30, 0, m_pattern.speed, m_pattern.color, 2);
    }
}

void BattleReact::spawnCross(SoulBox& soulBox) {
    float cx = soulBox.getSoulX();
    float cy = soulBox.getSoulY();

    soulBox.addBullet(cx - 30, cy, m_pattern.speed, 0, m_pattern.color, 4);
    soulBox.addBullet(cx + 30, cy, -m_pattern.speed, 0, m_pattern.color, 4);
    soulBox.addBullet(cx, cy - 30, 0, m_pattern.speed, m_pattern.color, 4);
    soulBox.addBullet(cx, cy + 30, 0, -m_pattern.speed, m_pattern.color, 4);
}