// ============================================================================
// NEON REQUIEM — Battle Reaction System (Header)
// ============================================================================
// REACTION-бои: игрок уклоняется от пуль в SOUL BOX.
// Генерирует паттерны пуль для разных врагов.
// ============================================================================

#ifndef NEON_REQUIEM_BATTLE_REACT_H
#define NEON_REQUIEM_BATTLE_REACT_H

#include <cstdint>
#include <Arduino.h>
#include "soul_box.h"
#include "config/config.h"

/// Паттерн атаки для REACTION боя
struct AttackPattern {
    uint8_t  type;        /// 0=спираль, 1=волна, 2=дождь, 3=крест
    uint8_t  bulletCount; /// Количество пуль
    float    speed;       /// Скорость пуль
    uint16_t color;       /// Цвет
    uint8_t  duration;    /// Длительность в кадрах
};

class BattleReact {
public:
    BattleReact();

    /// Инициализация атаки
    void beginAttack(AttackPattern pattern, SoulBox& soulBox);

    /// Обновление генерации пуль
    void update(SoulBox& soulBox);

    /// Проверить, завершена ли атака
    bool isAttackFinished() const { return m_finished; }

    /// Получить текущий паттерн
    const AttackPattern& getCurrentPattern() const { return m_pattern; }

private:
    /// Сгенерировать спираль пуль
    void spawnSpiral(SoulBox& soulBox);

    /// Сгенерировать волну пуль
    void spawnWave(SoulBox& soulBox);

    /// Сгенерировать дождь пуль
    void spawnRain(SoulBox& soulBox);

    /// Сгенерировать крест пуль
    void spawnCross(SoulBox& soulBox);

    AttackPattern m_pattern;
    int m_timer;
    int m_spawnTimer;
    bool m_finished;
};

#endif // NEON_REQUIEM_BATTLE_REACT_H