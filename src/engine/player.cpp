// ============================================================================
// NEON REQUIEM — Player (Implementation)
// ============================================================================
// Реализация игрока. Обрабатывает движение, анимацию, состояния,
// HP и RESONANCE-энергию.
// ============================================================================

#include "player.h"
#include "../core/input/input_manager.h"
#include "../core/display/display_manager.h"

// ============================================================================
// КОНСТРУКТОР
// ============================================================================

Player::Player()
    : m_x(0.0f)
    , m_y(0.0f)
    , m_speed(PLAYER_SPEED)
    , m_hp(PLAYER_MAX_HP)
    , m_maxHp(PLAYER_MAX_HP)
    , m_resonance(50)  // Начинаем с половиной энергии
    , m_state(PlayerState::IDLE)
    , m_direction(PlayerDirection::DOWN)
    , m_statusFlags(0)
    , m_moving(false)
    , m_animFrame(0)
    , m_animSpeed(4)
    , m_animTimer(0)
    , m_initialized(false)
{
}

// ============================================================================
// ИНИЦИАЛИЗАЦИЯ
// ============================================================================

void Player::begin() {
    // Стартовая позиция (центр карты)
    m_x = MAP_WIDTH_PX / 2.0f;
    m_y = MAP_HEIGHT_PX / 2.0f;

    m_hp = PLAYER_MAX_HP;
    m_resonance = 50;
    m_state = PlayerState::IDLE;
    m_direction = PlayerDirection::DOWN;

    m_initialized = true;
    LOG_INFO("Player: initialized at (%.1f, %.1f)", m_x, m_y);
}

// ============================================================================
// ОБНОВЛЕНИЕ
// ============================================================================

void Player::update() {
    if (!m_initialized) return;

    InputManager& input = InputManager::getInstance();

    // Обновление движения только в состоянии IDLE или WALKING
    if (m_state == PlayerState::IDLE || m_state == PlayerState::WALKING) {
        bool moved = false;

        // Проверяем D-Pad
        if (input.isPressed(Button::UP)) {
            move(PlayerDirection::UP);
            moved = true;
        } else if (input.isPressed(Button::DOWN)) {
            move(PlayerDirection::DOWN);
            moved = true;
        } else if (input.isPressed(Button::LEFT)) {
            move(PlayerDirection::LEFT);
            moved = true;
        } else if (input.isPressed(Button::RIGHT)) {
            move(PlayerDirection::RIGHT);
            moved = true;
        }

        if (!moved) {
            stop();
        }

        // RESONANCE (кнопка B)
        if (input.justPressed(Button::B)) {
            setState(PlayerState::RESONANCE);
        }
    }

    // Обновление анимации
    updateAnimation();

    // Восстановление RESONANCE (1 ед. в секунду)
    if (m_resonance < 100) {
        static uint32_t lastResonanceTick = 0;
        if (millis() - lastResonanceTick >= 1000) {
            m_resonance++;
            lastResonanceTick = millis();
        }
    }
}

// ============================================================================
// ОТРИСОВКА
// ============================================================================

void Player::render(int32_t cameraX, int32_t cameraY) {
    DisplayManager& display = DisplayManager::getInstance();

    // Позиция на экране с учётом камеры
    int32_t screenX = (int32_t)m_x - cameraX;
    int32_t screenY = (int32_t)m_y - cameraY;

    // TODO: отрисовка спрайта игрока (после создания SpriteRenderer)
    // Пока рисуем простой прямоугольник как placeholder

    int32_t playerSize = TILE_SIZE;

    // Цвет зависит от состояния
    uint16_t color;
    switch (m_state) {
        case PlayerState::RESONANCE:
            color = Palette::KAI_CORE;  // Свечение
            break;
        case PlayerState::HURT:
            color = Palette::RED;
            break;
        case PlayerState::STEALTH:
            color = Palette::TEXT_GRAY;
            break;
        default:
            color = Palette::KAI_COAT;
            break;
    }

    // Рисуем игрока
    display.fillRect(screenX, screenY, playerSize, playerSize, color);

    // Рисуем ядро (маленький квадрат в центре)
    if (m_state == PlayerState::RESONANCE) {
        display.fillRect(screenX + 4, screenY + 4,
                        playerSize - 8, playerSize - 8, Palette::CYAN);
    }

    // DEBUG: отрисовка направления
#if DEBUG_LEVEL >= 4
    uint16_t dirColor;
    switch (m_direction) {
        case PlayerDirection::UP:    dirColor = Palette::GREEN; break;
        case PlayerDirection::DOWN:  dirColor = Palette::RED; break;
        case PlayerDirection::LEFT:  dirColor = Palette::BLUE; break;
        case PlayerDirection::RIGHT: dirColor = Palette::YELLOW; break;
    }
    display.drawPixel(screenX + playerSize/2, screenY + playerSize/2, dirColor);
#endif
}

// ============================================================================
// ПОЗИЦИЯ
// ============================================================================

void Player::setPosition(float x, float y) {
    m_x = x;
    m_y = y;
}

void Player::setTilePosition(int tileX, int tileY) {
    m_x = tileX * TILE_SIZE + TILE_SIZE / 2.0f;
    m_y = tileY * TILE_SIZE + TILE_SIZE / 2.0f;
}

// ============================================================================
// ДВИЖЕНИЕ
// ============================================================================

void Player::move(PlayerDirection dir) {
    m_direction = dir;
    m_state = PlayerState::WALKING;
    m_moving = true;

    // Движение в зависимости от направления
    switch (dir) {
        case PlayerDirection::UP:
            m_y -= m_speed;
            break;
        case PlayerDirection::DOWN:
            m_y += m_speed;
            break;
        case PlayerDirection::LEFT:
            m_x -= m_speed;
            break;
        case PlayerDirection::RIGHT:
            m_x += m_speed;
            break;
    }

    // Ограничение границами карты
    m_x = CLAMP(m_x, 0.0f, (float)(MAP_WIDTH_PX - TILE_SIZE));
    m_y = CLAMP(m_y, 0.0f, (float)(MAP_HEIGHT_PX - TILE_SIZE));
}

void Player::stop() {
    m_moving = false;
    if (m_state == PlayerState::WALKING) {
        m_state = PlayerState::IDLE;
    }
}

void Player::teleport(float x, float y) {
    m_x = CLAMP(x, 0.0f, (float)(MAP_WIDTH_PX - TILE_SIZE));
    m_y = CLAMP(y, 0.0f, (float)(MAP_HEIGHT_PX - TILE_SIZE));
    LOG_INFO_F("Player: teleported to (%.1f, %.1f)", m_x, m_y);
}

// ============================================================================
// HP
// ============================================================================

void Player::setHP(int hp) {
    m_hp = CLAMP(hp, 0, m_maxHp);
}

void Player::heal(int amount) {
    m_hp = CLAMP(m_hp + amount, 0, m_maxHp);
    LOG_INFO_F("Player: healed +%d HP (%d/%d)", amount, m_hp, m_maxHp);
}

void Player::damage(int amount) {
    m_hp = CLAMP(m_hp - amount, 0, m_maxHp);
    m_state = PlayerState::HURT;
    LOG_INFO_F("Player: took %d damage (%d/%d)", amount, m_hp, m_maxHp);

    if (m_hp <= 0) {
        m_state = PlayerState::DEAD;
        LOG_INFO("Player: defeated!");
    }
}

// ============================================================================
// RESONANCE
// ============================================================================

void Player::setResonance(int value) {
    m_resonance = CLAMP(value, 0, 100);
}

void Player::addResonance(int amount) {
    m_resonance = CLAMP(m_resonance + amount, 0, 100);
}

bool Player::useResonance(int amount) {
    if (m_resonance >= amount) {
        m_resonance -= amount;
        return true;
    }
    return false;
}

// ============================================================================
// СОСТОЯНИЯ
// ============================================================================

void Player::setState(PlayerState state) {
    if (m_state == state) return;

    // Выход из текущего состояния
    switch (m_state) {
        case PlayerState::RESONANCE:
            // Возврат из RESONANCE
            break;
        default:
            break;
    }

    m_state = state;

    // Вход в новое состояние
    switch (state) {
        case PlayerState::RESONANCE:
            LOG_INFO("Player: entered RESONANCE mode");
            break;
        case PlayerState::STEALTH:
            LOG_INFO("Player: entered STEALTH mode");
            break;
        case PlayerState::HURT:
            // Автоматически выходим из HURT через 500 мс
            break;
        default:
            break;
    }
}

void Player::setDirection(PlayerDirection dir) {
    m_direction = dir;
}

// ============================================================================
// СТАТУСЫ
// ============================================================================

void Player::setStatus(PlayerStatus status, bool enabled) {
    if (enabled) {
        m_statusFlags |= static_cast<uint8_t>(status);
    } else {
        m_statusFlags &= ~static_cast<uint8_t>(status);
    }
}

bool Player::hasStatus(PlayerStatus status) const {
    return (m_statusFlags & static_cast<uint8_t>(status)) != 0;
}

// ============================================================================
// АНИМАЦИЯ
// ============================================================================

void Player::updateAnimation() {
    if (m_moving) {
        m_animTimer++;
        if (m_animTimer >= m_animSpeed) {
            m_animTimer = 0;
            m_animFrame = (m_animFrame + 1) % getAnimationFrameCount();
        }
    } else {
        m_animFrame = 0;
        m_animTimer = 0;
    }
}

int Player::getAnimationFrameCount() const {
    // 4 кадра анимации ходьбы
    return 4;
}

// ============================================================================
// СБРОС
// ============================================================================

void Player::reset() {
    m_x = MAP_WIDTH_PX / 2.0f;
    m_y = MAP_HEIGHT_PX / 2.0f;
    m_hp = PLAYER_MAX_HP;
    m_resonance = 50;
    m_state = PlayerState::IDLE;
    m_direction = PlayerDirection::DOWN;
    m_statusFlags = 0;
    m_moving = false;
    m_animFrame = 0;
    m_animTimer = 0;

    LOG_INFO("Player: reset to defaults");
}