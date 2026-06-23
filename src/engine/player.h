// ============================================================================
// NEON REQUIEM — Player (Header)
// ============================================================================
// Класс игрока (KAI). Содержит:
// - Позицию и движение по карте
// - HP, RESONANCE-энергию, статусы
// - Инвентарь (ссылка на Inventory)
// - Анимации (спрайты для 4 направлений)
// - Состояния (бег, диалог, RESONANCE, стелс)
// ============================================================================

#ifndef NEON_REQUIEM_PLAYER_H
#define NEON_REQUIEM_PLAYER_H

#include <cstdint>
#include <Arduino.h>
#include "config/config.h"
#include "config/palette.h"

// ============================================================================
// НАПРАВЛЕНИЯ
// ============================================================================

enum class PlayerDirection : uint8_t {
    DOWN  = 0,
    LEFT  = 1,
    RIGHT = 2,
    UP    = 3
};

// ============================================================================
// СОСТОЯНИЯ ИГРОКА
// ============================================================================

enum class PlayerState : uint8_t {
    IDLE      = 0,
    WALKING   = 1,
    TALKING   = 2,
    BATTLE    = 3,
    RESONANCE = 4,
    STEALTH   = 5,
    HURT      = 6,
    DEAD      = 7
};

// ============================================================================
// СТАТУСНЫЕ ЭФФЕКТЫ
// ============================================================================

enum class PlayerStatus : uint8_t {
    NONE       = 0,
    GLITCHED   = 1 << 0,  /// Искажение (GLITCH маршрут)
    CORRUPTED  = 1 << 1,  /// Повреждение данных
    BOOSTED    = 1 << 2,  /// Усиление RESONANCE
    TRACED     = 1 << 3   /// Отслеживание NEXUS
};

// ============================================================================
// PLAYER
// ============================================================================

class Player {
public:
    /// Singleton access
    static Player& getInstance() {
        static Player instance;
        return instance;
    }
    
    Player();
    ~Player() = default;

    /// Инициализация
    void begin();

    /// Обновление (движение, анимация)
    void update();

    /// Отрисовка на карте
    void render(int32_t cameraX, int32_t cameraY);

    // ========================================================================
    // ПОЗИЦИЯ И ДВИЖЕНИЕ
    // ========================================================================

    /// Установить позицию (в пикселях карты)
    void setPosition(float x, float y);

    /// Получить позицию
    float getX() const { return m_x; }
    float getY() const { return m_y; }

    /// Установить позицию в тайлах
    void setTilePosition(int tileX, int tileY);

    /// Получить позицию в тайлах
    int getTileX() const { return (int)(m_x / TILE_SIZE); }
    int getTileY() const { return (int)(m_y / TILE_SIZE); }

    /// Движение в направлении
    void move(PlayerDirection dir);

    /// Остановить движение
    void stop();

    /// Телепорт (мгновенное перемещение)
    void teleport(float x, float y);

    // ========================================================================
    // ХАРАКТЕРИСТИКИ
    // ========================================================================

    /// HP
    void setHP(int hp);
    int  getHP() const { return m_hp; }
    int  getMaxHP() const { return PLAYER_MAX_HP; }
    void heal(int amount);
    void damage(int amount);
    bool isAlive() const { return m_hp > 0; }

    /// RESONANCE энергия
    void setResonance(int value);
    int  getResonance() const { return m_resonance; }
    int  getMaxResonance() const { return 100; }
    void addResonance(int amount);
    bool useResonance(int amount);

    // ========================================================================
    // СОСТОЯНИЯ
    // ========================================================================

    void setState(PlayerState state);
    PlayerState getState() const { return m_state; }

    void setDirection(PlayerDirection dir);
    PlayerDirection getDirection() const { return m_direction; }

    /// Установить статусный эффект
    void setStatus(PlayerStatus status, bool enabled);

    /// Проверить статус
    bool hasStatus(PlayerStatus status) const;

    // ========================================================================
    // АНИМАЦИЯ
    // ========================================================================

    /// Получить текущий кадр анимации
    int getAnimationFrame() const { return m_animFrame; }

    /// Получить количество кадров анимации для текущего направления
    int getAnimationFrameCount() const;

    /// Установить скорость анимации (кадров на шаг)
    void setAnimationSpeed(int speed) { m_animSpeed = speed; }

    // ========================================================================
    // ПРОЧЕЕ
    // ========================================================================

    /// Проверить, движется ли игрок
    bool isMoving() const { return m_moving; }

    /// Получить скорость движения
    float getSpeed() const { return m_speed; }

    /// Установить скорость движения
    void setSpeed(float speed) { m_speed = speed; }

    /// Сбросить игрока (для новой игры)
    void reset();

private:
    /// Обновление анимации
    void updateAnimation();

    // Позиция (в пикселях карты, с плавающей точкой для плавности)
    float m_x;
    float m_y;

    // Скорость
    float m_speed;

    // Характеристики
    int m_hp;
    int m_maxHp;
    int m_resonance;

    // Состояния
    PlayerState     m_state;
    PlayerDirection m_direction;
    uint8_t         m_statusFlags;  /// Битовая маска статусов

    // Движение
    bool m_moving;

    // Анимация
    int  m_animFrame;
    int  m_animSpeed;
    int  m_animTimer;

    // Флаг инициализации
    bool m_initialized;
};

#endif // NEON_REQUIEM_PLAYER_H