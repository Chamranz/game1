// ============================================================================
// NEON REQUIEM — Input Manager (Implementation)
// ============================================================================
// Реализация системы ввода. Использует битовые маски для пакетного чтения
// GPIO, программный дебаунс и автоповтор с настраиваемыми задержками.
// ============================================================================

#include "input_manager.h"

// ============================================================================
// СИНГЛТОН
// ============================================================================

InputManager& InputManager::getInstance() {
    static InputManager instance;
    return instance;
}

// ============================================================================
// КОНСТРУКТОР
// ============================================================================

InputManager::InputManager()
    : m_lastRawState(0)
    , m_debouncedState(0)
    , m_debounceTimer(0)
    , m_initialized(false)
{
    // Обнуляем состояния кнопок
    for (int i = 0; i < 8; i++) {
        m_buttons[i] = {false, false, false, false, 0};
        m_repeatTimer[i] = 0;
        m_repeatActive[i] = false;
    }
}

// ============================================================================
// ИНИЦИАЛИЗАЦИЯ
// ============================================================================

bool InputManager::begin() {
    LOG_INFO("InputManager: initializing...");

    // Настраиваем пины D-Pad
    initPin(PIN_BTN_UP);
    initPin(PIN_BTN_DOWN);
    initPin(PIN_BTN_LEFT);
    initPin(PIN_BTN_RIGHT);

    // Настраиваем пины кнопок
    initPin(PIN_BTN_A);
    initPin(PIN_BTN_B);
    initPin(PIN_BTN_START);
    initPin(PIN_BTN_SELECT);

    // Читаем начальное состояние
    m_debouncedState = readAllButtons();
    m_lastRawState = m_debouncedState;

    m_initialized = true;
    LOG_INFO("InputManager: initialized successfully");
    return true;
}

// ============================================================================
// ИНИЦИАЛИЗАЦИЯ ПИНА
// ============================================================================

void InputManager::initPin(uint8_t pin) {
    // Кнопки подтянуты к GND через внешние резисторы 10kΩ.
    // Включаем внутренний pull-up для защиты от дребезга.
    pinMode(pin, INPUT_PULLUP);
}

// ============================================================================
// ЧТЕНИЕ ВСЕХ КНОПОК
// ============================================================================
// Читаем все 8 кнопок за один вызов через битовые маски.
// Это быстрее, чем читать каждую кнопку отдельно через digitalRead().

uint32_t InputManager::readAllButtons() {
    // Читаем GPIO регистр напрямую (GPIO_IN_REG)
    uint32_t gpio_state = GPIO.in1.data;

    // Инвертируем, т.к. кнопки — active LOW (0 = нажата)
    // Маскируем только нужные пины
    return (~gpio_state) & MASK_ALL_BUTTONS;
}

// ============================================================================
// ДЕБАУНС
// ============================================================================
// Простой программный дебаунс: состояние меняется только если оно
// стабильно в течение 2 последовательных чтений.

uint32_t InputManager::debounce(uint32_t raw) {
    // Если состояние совпадает с прошлым — считаем стабильным
    if (raw == m_lastRawState) {
        return raw;
    }

    // Если не совпадает — запоминаем и ждём следующего кадра
    m_lastRawState = raw;
    return m_debouncedState;
}

// ============================================================================
// ОБНОВЛЕНИЕ
// ============================================================================

void InputManager::update() {
    if (!m_initialized) return;

    // 1. Читаем сырое состояние GPIO
    uint32_t raw = readAllButtons();

    // 2. Дебаунс
    uint32_t current = debounce(raw);

    // 3. Определяем изменения
    uint32_t changed = current ^ m_debouncedState;
    uint32_t pressed = current & changed;    // Было 0, стало 1
    uint32_t released = (~current) & changed; // Было 1, стало 0

    // 4. Обновляем состояния кнопок
    for (int i = 0; i < 8; i++) {
        Button btn = static_cast<Button>(i);
        uint32_t mask = buttonToMask(btn);

        m_buttons[i].pressed = (current & mask) != 0;
        m_buttons[i].justPressed = (pressed & mask) != 0;
        m_buttons[i].justReleased = (released & mask) != 0;

        // Сброс justPressed/justReleased после обработки
        if (m_buttons[i].justPressed) {
            m_repeatTimer[i] = 0;
            m_repeatActive[i] = false;
        }
    }

    // 5. Обновляем автоповтор
    updateAutoRepeat();

    // 6. Сохраняем текущее состояние
    m_debouncedState = current;
}

// ============================================================================
// АВТОПОВТОР
// ============================================================================
// Реализация автоповтора для навигации по меню и диалогам.
// Первая задержка — BUTTON_REPEAT_INIT_MS, затем каждые BUTTON_REPEAT_DELAY_MS.

void InputManager::updateAutoRepeat() {
    for (int i = 0; i < 8; i++) {
        if (!m_buttons[i].pressed) {
            m_repeatTimer[i] = 0;
            m_repeatActive[i] = false;
            m_buttons[i].held = false;
            continue;
        }

        // Кнопка удерживается
        m_repeatTimer[i] += FRAME_TIME_MS;

        if (!m_repeatActive[i]) {
            // Ждём первую задержку
            if (m_repeatTimer[i] >= BUTTON_REPEAT_INIT_MS) {
                m_repeatActive[i] = true;
                m_repeatTimer[i] = 0;
                m_buttons[i].held = true;
            }
        } else {
            // Автоповтор с интервалом
            if (m_repeatTimer[i] >= BUTTON_REPEAT_DELAY_MS) {
                m_repeatTimer[i] = 0;
                m_buttons[i].held = true;
            } else {
                m_buttons[i].held = false;
            }
        }
    }
}

// ============================================================================
// ПРОВЕРКИ СОСТОЯНИЯ
// ============================================================================

bool InputManager::isPressed(Button btn) const {
    return m_buttons[static_cast<int>(btn)].pressed;
}

bool InputManager::justPressed(Button btn) const {
    return m_buttons[static_cast<int>(btn)].justPressed;
}

bool InputManager::justReleased(Button btn) const {
    return m_buttons[static_cast<int>(btn)].justReleased;
}

bool InputManager::isHeld(Button btn) const {
    return m_buttons[static_cast<int>(btn)].held;
}

// ============================================================================
// НАПРАВЛЕНИЕ D-PAD
// ============================================================================

int8_t InputManager::getDPadDirection() const {
    bool up    = m_buttons[0].pressed;
    bool down  = m_buttons[1].pressed;
    bool left  = m_buttons[2].pressed;
    bool right = m_buttons[3].pressed;

    // Приоритет: вертикаль > горизонталь
    if (up && left)    return 7;  // UP-LEFT
    if (up && right)   return 1;  // UP-RIGHT
    if (down && left)  return 5;  // DOWN-LEFT
    if (down && right) return 3;  // DOWN-RIGHT
    if (up)            return 0;  // UP
    if (down)          return 4;  // DOWN
    if (left)          return 6;  // LEFT
    if (right)         return 2;  // RIGHT

    return -1;  // Ничего не нажато
}

// ============================================================================
// ОЖИДАНИЕ НАЖАТИЯ
// ============================================================================

Button InputManager::waitForPress(uint32_t timeout_ms) {
    uint32_t start = millis();

    while (true) {
        update();

        for (int i = 0; i < 8; i++) {
            if (m_buttons[i].justPressed) {
                return static_cast<Button>(i);
            }
        }

        // Проверка таймаута
        if (timeout_ms > 0 && (millis() - start) >= timeout_ms) {
            break;
        }

        // Небольшая задержка для снижения нагрузки
        delay(1);
    }

    return Button::NONE;
}

// ============================================================================
// СБРОС
// ============================================================================

void InputManager::reset() {
    for (int i = 0; i < 8; i++) {
        m_buttons[i] = {false, false, false, false, 0};
        m_repeatTimer[i] = 0;
        m_repeatActive[i] = false;
    }
    m_debouncedState = 0;
    m_lastRawState = 0;
}

// ============================================================================
// СЫРОЕ СОСТОЯНИЕ GPIO
// ============================================================================

uint32_t InputManager::getRawGPIO(){
    return readAllButtons();
}

// ============================================================================
// МАППИНГИ
// ============================================================================

Button InputManager::gpioToButton(uint8_t gpio) const {
    switch (gpio) {
        case PIN_BTN_UP:     return Button::UP;
        case PIN_BTN_DOWN:   return Button::DOWN;
        case PIN_BTN_LEFT:   return Button::LEFT;
        case PIN_BTN_RIGHT:  return Button::RIGHT;
        case PIN_BTN_A:      return Button::A;
        case PIN_BTN_B:      return Button::B;
        case PIN_BTN_START:  return Button::START;
        case PIN_BTN_SELECT: return Button::SELECT;
        default:             return Button::NONE;
    }
}

uint32_t InputManager::buttonToMask(Button btn) const {
    switch (btn) {
        case Button::UP:     return (1ULL << PIN_BTN_UP);
        case Button::DOWN:   return (1ULL << PIN_BTN_DOWN);
        case Button::LEFT:   return (1ULL << PIN_BTN_LEFT);
        case Button::RIGHT:  return (1ULL << PIN_BTN_RIGHT);
        case Button::A:      return (1ULL << PIN_BTN_A);
        case Button::B:      return (1ULL << PIN_BTN_B);
        case Button::START:  return (1ULL << PIN_BTN_START);
        case Button::SELECT: return (1ULL << PIN_BTN_SELECT);
        default:             return 0;
    }
}