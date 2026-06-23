// ============================================================================
// NEON REQUIEM — Soul Box (Header)
// ============================================================================
// SOUL BOX — зона уклонения в бою. Сердце (SOUL) игрока перемещается
// внутри прямоугольной области, уклоняясь от пуль противника.
// ============================================================================

#ifndef NEON_REQUIEM_SOUL_BOX_H
#define NEON_REQUIEM_SOUL_BOX_H

#include <cstdint>
#include <Arduino.h>
#include "config/config.h"

/// Пуля в bullet-hell режиме
struct Bullet {
    float x, y;        /// Позиция
    float vx, vy;      /// Скорость
    uint16_t color;    /// Цвет
    uint8_t size;      /// Размер
    bool active;       /// Активна?
};

class SoulBox {
public:
    SoulBox();

    /// Инициализация
    void begin(int boxX, int boxY, int boxW, int boxH);

    /// Обновление (движение SOUL и пуль)
    void update();

    /// Отрисовка
    void render();

    /// Установить позицию SOUL
    void setSoulPosition(float x, float y);

    /// Получить позицию SOUL
    float getSoulX() const { return m_soulX; }
    float getSoulY() const { return m_soulY; }

    /// Добавить пулю
    void addBullet(float x, float y, float vx, float vy,
                   uint16_t color = 0xFF0000, uint8_t size = 3);

    /// Очистить все пули
    void clearBullets();

    /// Проверить попадание в SOUL
    bool isHit() const;

    /// Получить количество активных пуль
    int getActiveBulletCount() const;

    /// Установить скорость SOUL
    void setSoulSpeed(float speed) { m_soulSpeed = speed; }

private:
    /// Проверка коллизии SOUL с пулей
    bool checkCollision(const Bullet& bullet) const;

    // SOUL
    float m_soulX, m_soulY;
    float m_soulSpeed;

    // Box
    int m_boxX, m_boxY, m_boxW, m_boxH;

    // Пули
    Bullet m_bullets[MAX_BULLETS];
    int    m_bulletCount;

    // Состояние
    bool m_hit;
    int  m_hitTimer;
};

#endif // NEON_REQUIEM_SOUL_BOX_H