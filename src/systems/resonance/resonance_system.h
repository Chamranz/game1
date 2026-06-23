// ============================================================================
// NEON REQUIEM — Resonance System (Header)
// ============================================================================
// RESONANCE — способность Кая видеть цифровую сущность вещей.
// Подсвечивает скрытые объекты, пути, слабости врагов.
// ============================================================================

#ifndef NEON_REQUIEM_RESONANCE_SYSTEM_H
#define NEON_REQUIEM_RESONANCE_SYSTEM_H

#include <cstdint>
#include <Arduino.h>
#include "config/config.h"

struct ResonanceObject {
    int16_t  x, y, w, h;
    uint16_t color;
    char     info[64];
    bool     active;
};

class ResonanceSystem {
public:
    ResonanceSystem();

    void begin();
    void update();
    void render(int32_t cameraX, int32_t cameraY);

    void activate();
    void deactivate();
    bool isActive() const { return m_active; }

    void addObject(int16_t x, int16_t y, int16_t w, int16_t h,
                   uint16_t color, const char* info);
    void clearObjects();

    int  getEnergyCost() const { return 5; }
    int  getEnergyDrain() const { return 1; }

private:
    ResonanceObject m_objects[16];
    int m_objectCount;
    bool m_active;
    int m_energyTimer;
    uint32_t m_pulseTimer;
};

#endif