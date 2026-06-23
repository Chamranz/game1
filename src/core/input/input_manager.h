// ============================================================================
// NEON REQUIEM — Input Manager (Header)
// ============================================================================
// Система ввода с аппаратным дебаунсом, автоповтором и поддержкой
// прерываний. Обрабатывает D-Pad и 4 кнопки (A, B, START, SELECT).
// Использует битовые маски для пакетного чтения GPIO.
// ============================================================================

#ifndef NEON_REQUIEM_INPUT_MANAGER_H
#define NEON_REQUIEM_INPUT_MANAGER_H

#include <cstdint>
#include <Arduino.h>
#include "config/config.h"
#include "config/pins.h"

// ============================================================================
// ТИПЫ КНОПОК
// ============================================================================

/// Перечисление всех кнопок
enum class Button : uint8_t {
    UP     = 0,
    DOWN   = 1,
    LEFT   = 2,
    RIGHT  = 3,
    A      = 4,
    B      = 5,
    START  = 6,
    SELECT = 7,
    NONE   = 8
};

/// Состояние кнопки
struct ButtonState {
    bool pressed     : 1;  /// Нажата в данный момент
    bool justPressed : 1;  /// Нажата в этом кадре (фронт)
    bool justReleased: 1;  /// Отпущена в этом кадре
    bool held        : 1;  /// Удерживается (после автоповтора)
    uint8_t holdTime : 4;  /// Время удержания в кадрах (0-15)
};

// ============================================================================
// INPUT MANAGER
// ============================================================================

class InputManager {
public:
    /// Получить единственный экземпляр (синглтон)
    static InputManager& getInstance();

    /// Инициализация пинов и прерываний
    /// @return true если успешно
    bool begin();

    /// Обновление состояния кнопок (вызывать каждый кадр)
    void update();

    /// Проверка, нажата ли кнопка в данный момент
    bool isPressed(Button btn) const;

    /// Проверка, нажата ли кнопка в этом кадре (однократно)
    bool justPressed(Button btn) const;

    /// Проверка, отпущена ли кнопка в этом кадре
    bool justReleased(Button btn) const;

    /// Проверка, удерживается ли кнопка (автоповтор)
    bool isHeld(Button btn) const;

    /// Получить направление D-Pad
    /// @return 0-7 (0=UP, 1=UP-RIGHT, 2=RIGHT, ... 7=UP-LEFT), -1 если ничего
    int8_t getDPadDirection() const;

    /// Ожидание нажатия любой кнопки (блокирующее)
    /// @param timeout_ms — таймаут в мс (0 = бесконечно)
    /// @return какая кнопка была нажата, или Button::NONE
    Button waitForPress(uint32_t timeout_ms = 0);

    /// Сбросить все состояния
    void reset();

    /// Получить сырое состояние GPIO (для отладки)
    uint32_t getRawGPIO();

private:
    // Приватный конструктор (синглтон)
    InputManager();
    ~InputManager() = default;
    InputManager(const InputManager&) = delete;
    InputManager& operator=(const InputManager&) = delete;

    /// Инициализация конкретного пина
    void initPin(uint8_t pin);

    /// Чтение состояния всех кнопок из GPIO
    uint32_t readAllButtons();

    /// Применение дебаунса
    uint32_t debounce(uint32_t raw);

    /// Обновление автоповтора
    void updateAutoRepeat();

    /// Маппинг битовой маски на Button
    Button gpioToButton(uint8_t gpio) const;

    /// Маппинг Button на битовую маску
    uint32_t buttonToMask(Button btn) const;

    // Состояния кнопок
    ButtonState m_buttons[8];  /// Состояния всех 8 кнопок

    // Дебаунс
    uint32_t m_lastRawState;   /// Сырое состояние в прошлом кадре
    uint32_t m_debouncedState; /// Отфильтрованное состояние
    uint32_t m_debounceTimer;  /// Таймер дебаунса

    // Автоповтор
    uint32_t m_repeatTimer[8]; /// Таймеры автоповтора для каждой кнопки
    bool     m_repeatActive[8];/// Флаг: автоповтор активирован

    // Флаги
    bool m_initialized;
};

#endif // NEON_REQUIEM_INPUT_MANAGER_H