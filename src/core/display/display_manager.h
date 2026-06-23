// ============================================================================
// NEON REQUIEM — Display Manager (Header)
// ============================================================================
// Управление TFT-дисплеем ST7789 320x240. Обеспечивает:
// - Инициализацию дисплея с правильной распиновкой
// - Двойную буферизацию (кадровый буфер в PSRAM)
// - Базовые примитивы (пиксели, линии, прямоугольники, круги)
// - Работу с областями экрана (viewport)
// - Аппаратный скроллинг
// - Управление подсветкой (PWM)
// ============================================================================

#ifndef NEON_REQUIEM_DISPLAY_MANAGER_H
#define NEON_REQUIEM_DISPLAY_MANAGER_H

#include <cstdint>
#include <Arduino.h>
#include <TFT_eSPI.h>
#include "config/config.h"
#include "config/pins.h"
#include "config/palette.h"

// ============================================================================
// FRAMEBUFFER
// ============================================================================

/// Кадровый буфер в PSRAM (320x240 пикселей, 16-bit = 153600 байт)
/// Используем PSRAM, т.к. внутренней RAM ESP32-S3 недостаточно.
struct FrameBuffer {
    uint16_t* buffer;       /// Указатель на буфер в PSRAM
    bool      usePSRAM;     /// Флаг: буфер в PSRAM
    int32_t   width;        /// Ширина буфера
    int32_t   height;       /// Высота буфера
    int32_t   stride;       /// Шаг (ширина в пикселях)
};

// ============================================================================
// DISPLAY MANAGER
// ============================================================================

class DisplayManager {
public:
    /// Получить единственный экземпляр (синглтон)
    static DisplayManager& getInstance();

    uint16_t readPixel(int32_t x, int32_t y) { return m_tft.readPixel(x, y); }
    void fillScreen(uint16_t color) { m_tft.fillScreen(color); }

    /// Инициализация дисплея
    /// @return true если успешно
    bool begin();

    /// Очистить экран (залить цветом)
    void clear(uint16_t color = Palette::DEEP_BG);

    /// Начать кадр (переключить буферы)
    void beginFrame();

    /// Завершить кадр (вывести на экран)
    void endFrame();

    /// Установить яркость подсветки (0-255)
    void setBrightness(uint8_t brightness);

    /// Получить яркость подсветки
    uint8_t getBrightness() const;

    // ========================================================================
    // ПРИМИТИВЫ
    // ========================================================================

    /// Нарисовать пиксель
    void drawPixel(int32_t x, int32_t y, uint16_t color);

    /// Нарисовать линию
    void drawLine(int32_t x0, int32_t y0, int32_t x1, int32_t y1, uint16_t color);

    /// Нарисовать залитый прямоугольник
    void fillRect(int32_t x, int32_t y, int32_t w, int32_t h, uint16_t color);

    /// Нарисовать прямоугольник (только рамка)
    void drawRect(int32_t x, int32_t y, int32_t w, int32_t h, uint16_t color);

    /// Нарисовать залитый круг
    void fillCircle(int32_t x, int32_t y, int32_t r, uint16_t color);

    /// Нарисовать окружность
    void drawCircle(int32_t x, int32_t y, int32_t r, uint16_t color);

    /// Нарисовать треугольник (залитый)
    void fillTriangle(int32_t x0, int32_t y0, int32_t x1, int32_t y1,
                      int32_t x2, int32_t y2, uint16_t color);

    // ========================================================================
    // РАБОТА С ОБЛАСТЯМИ (VIEWPORT)
    // ========================================================================

    /// Установить область рисования (viewport)
    void setViewport(int32_t x, int32_t y, int32_t w, int32_t h);

    /// Сбросить viewport на весь экран
    void resetViewport();

    /// Получить текущий viewport
    void getViewport(int32_t& x, int32_t& y, int32_t& w, int32_t& h) const;

    // ========================================================================
    // РАБОТА С БУФЕРОМ
    // ========================================================================

    /// Получить указатель на кадровый буфер
    uint16_t* getBuffer();

    /// Получить размер буфера в байтах
    size_t getBufferSize() const;

    /// Очистить буфер (без вывода на экран)
    void clearBuffer(uint16_t color = Palette::DEEP_BG);

    /// Скопировать область буфера на экран
    void flushRegion(int32_t x, int32_t y, int32_t w, int32_t h);

    /// Прямой доступ к пикселю в буфере
    void setBufferPixel(int32_t x, int32_t y, uint16_t color);

    /// Чтение пикселя из буфера
    uint16_t getBufferPixel(int32_t x, int32_t y) const;

    // ========================================================================
    // ЭФФЕКТЫ
    // ========================================================================

    /// Заполнить экран шумом (для CRT-эффекта)
    void fillNoise(uint16_t color1, uint16_t color2, uint8_t density);

    /// Горизонтальный скролл буфера
    void scrollHorizontal(int32_t offset);

    /// Вертикальный скролл буфера
    void scrollVertical(int32_t offset);

    // ========================================================================
    // ДОСТУП К TFT
    // ========================================================================

    /// Получить ссылку на TFT_eSPI (для продвинутого использования)
    TFT_eSPI& getTFT();

private:
    DisplayManager();
    ~DisplayManager() = default;
    DisplayManager(const DisplayManager&) = delete;
    DisplayManager& operator=(const DisplayManager&) = delete;

    /// Инициализация подсветки (PWM)
    void initBacklight();

    /// Выделение кадрового буфера
    bool allocateFramebuffer();

    /// Освобождение кадрового буфера
    void freeFramebuffer();

    /// Проверка координат в пределах экрана
    bool isInBounds(int32_t x, int32_t y) const;

    /// Проверка координат в пределах viewport
    bool isInViewport(int32_t x, int32_t y) const;

    // TFT объект
    TFT_eSPI m_tft;

    // Кадровый буфер
    FrameBuffer m_framebuffer;

    // Viewport
    int32_t m_viewX, m_viewY, m_viewW, m_viewH;

    // Подсветка
    uint8_t m_brightness;
    uint8_t m_backlightPin;

    // Состояние
    bool m_initialized;
    bool m_frameActive;  /// Флаг: между beginFrame() и endFrame()
};

#endif // NEON_REQUIEM_DISPLAY_MANAGER_H