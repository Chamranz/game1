// ============================================================================
// NEON REQUIEM — Soul Box (Implementation)
// ============================================================================

#include "soul_box.h"
#include "../../core/input/input_manager.h"
#include "../../core/display/display_manager.h"

SoulBox::SoulBox()
    : m_soulX(0), m_soulY(0), m_soulSpeed(2.0f)
    , m_boxX(0), m_boxY(0), m_boxW(SOUL_BOX_WIDTH), m_boxH(SOUL_BOX_HEIGHT)
    , m_bulletCount(0), m_hit(false), m_hitTimer(0)
{
    for (int i = 0; i < MAX_BULLETS; i++) {
        m_bullets[i].active = false;
    }
}

void SoulBox::begin(int boxX, int boxY, int boxW, int boxH) {
    m_boxX = boxX; m_boxY = boxY; m_boxW = boxW; m_boxH = boxH;
    m_soulX = boxX + boxW / 2.0f;
    m_soulY = boxY + boxH / 2.0f;
    m_hit = false;
}

void SoulBox::update() {
    InputManager& input = InputManager::getInstance();

    // Движение SOUL
    if (input.isPressed(Button::UP))    m_soulY -= m_soulSpeed;
    if (input.isPressed(Button::DOWN))  m_soulY += m_soulSpeed;
    if (input.isPressed(Button::LEFT))  m_soulX -= m_soulSpeed;
    if (input.isPressed(Button::RIGHT)) m_soulX += m_soulSpeed;

    // Границы
    m_soulX = CLAMP(m_soulX, m_boxX + 3.0f, m_boxX + m_boxW - 3.0f);
    m_soulY = CLAMP(m_soulY, m_boxY + 3.0f, m_boxY + m_boxH - 3.0f);

    // Обновление пуль
    for (int i = 0; i < m_bulletCount; i++) {
        if (!m_bullets[i].active) continue;

        m_bullets[i].x += m_bullets[i].vx;
        m_bullets[i].y += m_bullets[i].vy;

        // Деактивируем пули за пределами box
        if (m_bullets[i].x < m_boxX - 10 || m_bullets[i].x > m_boxX + m_boxW + 10 ||
            m_bullets[i].y < m_boxY - 10 || m_bullets[i].y > m_boxY + m_boxH + 10) {
            m_bullets[i].active = false;
        }

        // Проверка коллизии
        if (!m_hit && checkCollision(m_bullets[i])) {
            m_hit = true;
            m_hitTimer = 30;  // 1 секунда неуязвимости
        }
    }

    // Таймер неуязвимости
    if (m_hitTimer > 0) {
        m_hitTimer--;
        if (m_hitTimer == 0) m_hit = false;
    }
}

void SoulBox::render() {
    DisplayManager& display = DisplayManager::getInstance();

    // Рамка
    display.drawRect(m_boxX, m_boxY, m_boxW, m_boxH, Palette::TEXT_WHITE);

    // Пули
    for (int i = 0; i < m_bulletCount; i++) {
        if (!m_bullets[i].active) continue;
        display.fillRect(m_bullets[i].x, m_bullets[i].y,
                        m_bullets[i].size, m_bullets[i].size, m_bullets[i].color);
    }

    // SOUL (сердце)
    if (!m_hit || (m_hitTimer / 3) % 2 == 0) {  // Мигание при попадании
        display.fillRect(m_soulX - 3, m_soulY - 3, 6, 6, Palette::HP_FULL);
        display.drawRect(m_soulX - 3, m_soulY - 3, 6, 6, Palette::WHITE);
    }
}

void SoulBox::setSoulPosition(float x, float y) {
    m_soulX = x; m_soulY = y;
}

void SoulBox::addBullet(float x, float y, float vx, float vy, uint16_t color, uint8_t size) {
    if (m_bulletCount >= MAX_BULLETS) return;

    m_bullets[m_bulletCount] = {x, y, vx, vy, color, size, true};
    m_bulletCount++;
}

void SoulBox::clearBullets() {
    for (int i = 0; i < m_bulletCount; i++) {
        m_bullets[i].active = false;
    }
    m_bulletCount = 0;
}

bool SoulBox::isHit() const { return m_hit; }

int SoulBox::getActiveBulletCount() const {
    int count = 0;
    for (int i = 0; i < m_bulletCount; i++) {
        if (m_bullets[i].active) count++;
    }
    return count;
}

bool SoulBox::checkCollision(const Bullet& bullet) const {
    float dx = m_soulX - bullet.x;
    float dy = m_soulY - bullet.y;
    float dist = sqrt(dx * dx + dy * dy);
    return dist < (3 + bullet.size);
}