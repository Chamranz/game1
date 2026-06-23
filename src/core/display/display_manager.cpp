// ============================================================================
// NEON REQUIEM — Display Manager (Implementation)
// ============================================================================
// Реализация управления дисплеем. Использует двойную буферизацию через
// PSRAM для плавной 30 FPS анимации без мерцания.
// ============================================================================

#include "display_manager.h"
#define swap(a, b) do { auto temp = a; a = b; b = temp; } while(0)

// ============================================================================
// СИНГЛТОН
// ============================================================================

DisplayManager& DisplayManager::getInstance() {
    static DisplayManager instance;
    return instance;
}

// ============================================================================
// КОНСТРУКТОР
// ============================================================================

DisplayManager::DisplayManager()
    : m_brightness(255)
    , m_backlightPin(PIN_TFT_BL)
    , m_initialized(false)
    , m_frameActive(false)
{
    m_framebuffer.buffer   = nullptr;
    m_framebuffer.usePSRAM = false;
    m_framebuffer.width    = DISPLAY_WIDTH;
    m_framebuffer.height   = DISPLAY_HEIGHT;
    m_framebuffer.stride   = DISPLAY_WIDTH;

    // Viewport по умолчанию — весь экран
    m_viewX = 0;
    m_viewY = 0;
    m_viewW = DISPLAY_WIDTH;
    m_viewH = DISPLAY_HEIGHT;
}

// ============================================================================
// ИНИЦИАЛИЗАЦИЯ
// ============================================================================

bool DisplayManager::begin() {
    LOG_INFO("DisplayManager: initializing ST7789...");

    // 1. Инициализация подсветки
    initBacklight();

    // 2. Инициализация TFT
    m_tft.begin();
    m_tft.setRotation(0);  // Портретная ориентация (320x240)

    // 3. Включаем дисплей
    m_tft.writecommand(TFT_SLPOUT);  // Выход из sleep
    delay(120);
    m_tft.writecommand(TFT_DISPON);  // Включаем дисплей

    // 4. Выделяем кадровый буфер
    if (!allocateFramebuffer()) {
        LOG_ERROR("DisplayManager: failed to allocate framebuffer!");
        return false;
    }

    // 5. Очищаем буфер
    clearBuffer(Palette::DEEP_BG);
    flushRegion(0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT);

    m_initialized = true;
    LOG_INFO("DisplayManager: initialized successfully");
    return true;
}

// ============================================================================
// ПОДСВЕТКА (PWM)
// ============================================================================

void DisplayManager::initBacklight() {
    // Настраиваем PWM на пине подсветки
    // Частота ШИМ: 5000 Гц (достаточно для подсветки, без мерцания)
    // Разрядность: 8 бит (0-255)
    // ledcAttachPin(pin, channel) — 2 аргумента в ESP32 Arduino Core
    ledcAttachPin(m_backlightPin, 0);
    ledcSetup(0, 5000, 8);
    ledcWrite(0, m_brightness);
}

void DisplayManager::setBrightness(uint8_t brightness) {
    m_brightness = brightness;
    ledcWrite(0, m_brightness);
}

uint8_t DisplayManager::getBrightness() const {
    return m_brightness;
}

// ============================================================================
// КАДРОВЫЙ БУФЕР
// ============================================================================

bool DisplayManager::allocateFramebuffer() {
    size_t bufferSize = DISPLAY_WIDTH * DISPLAY_HEIGHT * sizeof(uint16_t);
    LOG_INFO_F("DisplayManager: allocating %d bytes for framebuffer", bufferSize);

    // Пытаемся выделить в PSRAM
#if USE_PSRAM_SPRITES
    if (psramFound()) {
        m_framebuffer.buffer = (uint16_t*)ps_malloc(bufferSize);
        m_framebuffer.usePSRAM = true;
        LOG_INFO("DisplayManager: framebuffer in PSRAM");
    } else
#endif
    {
        // Fallback на внутреннюю RAM (может не хватить)
        m_framebuffer.buffer = (uint16_t*)malloc(bufferSize);
        m_framebuffer.usePSRAM = false;
        LOG_WARN("DisplayManager: PSRAM not found, using internal RAM");
    }

    return m_framebuffer.buffer != nullptr;
}

void DisplayManager::freeFramebuffer() {
    if (m_framebuffer.buffer) {
        free(m_framebuffer.buffer);
        m_framebuffer.buffer = nullptr;
    }
}

// ============================================================================
// КАДРЫ
// ============================================================================

void DisplayManager::beginFrame() {
    m_frameActive = true;
}

void DisplayManager::endFrame() {
    if (!m_frameActive) return;

    // Копируем весь буфер на дисплей через SPI DMA
    m_tft.pushImage(0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, m_framebuffer.buffer);

    m_frameActive = false;
}

// ============================================================================
// ОЧИСТКА
// ============================================================================

void DisplayManager::clear(uint16_t color) {
    clearBuffer(color);
    flushRegion(0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT);
}

void DisplayManager::clearBuffer(uint16_t color) {
    if (!m_framebuffer.buffer) return;

    // Быстрая заливка через memset32 (в 4 раза быстрее цикла)
    uint32_t color32 = (color << 16) | color;
    size_t words = (DISPLAY_WIDTH * DISPLAY_HEIGHT) / 2;
    uint32_t* buf32 = (uint32_t*)m_framebuffer.buffer;

    for (size_t i = 0; i < words; i++) {
        buf32[i] = color32;
    }
}

// ============================================================================
// ПРИМИТИВЫ
// ============================================================================

void DisplayManager::drawPixel(int32_t x, int32_t y, uint16_t color) {
    if (!m_framebuffer.buffer || !isInViewport(x, y)) return;
    setBufferPixel(x, y, color);
}

void DisplayManager::drawLine(int32_t x0, int32_t y0, int32_t x1, int32_t y1, uint16_t color) {
    // Алгоритм Брезенхема
    int32_t dx = abs(x1 - x0);
    int32_t dy = -abs(y1 - y0);
    int32_t sx = (x0 < x1) ? 1 : -1;
    int32_t sy = (y0 < y1) ? 1 : -1;
    int32_t err = dx + dy;
    int32_t e2;

    while (true) {
        drawPixel(x0, y0, color);
        if (x0 == x1 && y0 == y1) break;
        e2 = 2 * err;
        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y0 += sy; }
    }
}

void DisplayManager::fillRect(int32_t x, int32_t y, int32_t w, int32_t h, uint16_t color) {
    if (!m_framebuffer.buffer) return;

    // Обрезаем по viewport
    int32_t x0 = max(x, m_viewX);
    int32_t y0 = max(y, m_viewY);
    int32_t x1 = min(x + w, m_viewX + m_viewW);
    int32_t y1 = min(y + h, m_viewY + m_viewH);

    if (x0 >= x1 || y0 >= y1) return;

    // Быстрая заливка строк
    for (int32_t row = y0; row < y1; row++) {
        int32_t idx = row * m_framebuffer.stride + x0;
        for (int32_t col = x0; col < x1; col++) {
            m_framebuffer.buffer[idx++] = color;
        }
    }
}

void DisplayManager::drawRect(int32_t x, int32_t y, int32_t w, int32_t h, uint16_t color) {
    drawLine(x, y, x + w - 1, y, color);           // Верх
    drawLine(x, y + h - 1, x + w - 1, y + h - 1, color); // Низ
    drawLine(x, y, x, y + h - 1, color);           // Лево
    drawLine(x + w - 1, y, x + w - 1, y + h - 1, color); // Право
}

void DisplayManager::fillCircle(int32_t x, int32_t y, int32_t r, uint16_t color) {
    // Алгоритм Midpoint Circle (залитый)
    int32_t cx = 0, cy = r;
    int32_t d = 3 - 2 * r;

    while (cy >= cx) {
        drawLine(x - cx, y - cy, x + cx, y - cy, color);
        drawLine(x - cy, y - cx, x + cy, y - cx, color);
        drawLine(x - cx, y + cy, x + cx, y + cy, color);
        drawLine(x - cy, y + cx, x + cy, y + cx, color);
        cx++;
        if (d > 0) {
            cy--;
            d = d + 4 * (cx - cy) + 10;
        } else {
            d = d + 4 * cx + 6;
        }
    }
}

void DisplayManager::drawCircle(int32_t x, int32_t y, int32_t r, uint16_t color) {
    // Алгоритм Midpoint Circle (контур)
    int32_t cx = 0, cy = r;
    int32_t d = 3 - 2 * r;

    while (cy >= cx) {
        drawPixel(x + cx, y + cy, color);
        drawPixel(x - cx, y + cy, color);
        drawPixel(x + cx, y - cy, color);
        drawPixel(x - cx, y - cy, color);
        drawPixel(x + cy, y + cx, color);
        drawPixel(x - cy, y + cx, color);
        drawPixel(x + cy, y - cx, color);
        drawPixel(x - cy, y - cx, color);
        cx++;
        if (d > 0) {
            cy--;
            d = d + 4 * (cx - cy) + 10;
        } else {
            d = d + 4 * cx + 6;
        }
    }
}

void DisplayManager::fillTriangle(int32_t x0, int32_t y0, int32_t x1, int32_t y1,
                                   int32_t x2, int32_t y2, uint16_t color) {
    // Сортировка по Y
    if (y0 > y1) { swap(x0, x1); swap(y0, y1); }
    if (y1 > y2) { swap(x1, x2); swap(y1, y2); }
    if (y0 > y1) { swap(x0, x1); swap(y0, y1); }

    auto drawScanline = [&](int32_t y, int32_t xa, int32_t xb) {
        if (xa > xb) swap(xa, xb);
        for (int32_t x = xa; x <= xb; x++) drawPixel(x, y, color);
    };

    float dx1 = (float)(x1 - x0) / (y1 - y0);
    float dx2 = (float)(x2 - x0) / (y2 - y0);
    float dx3 = (float)(x2 - x1) / (y2 - y1);

    float xa = x0, xb = x0;
    for (int32_t y = y0; y <= y1; y++) {
        drawScanline(y, xa, xb);
        xa += dx1;
        xb += dx2;
    }

    xa = x1;
    for (int32_t y = y1; y <= y2; y++) {
        drawScanline(y, xa, xb);
        xa += dx3;
        xb += dx2;
    }
}

// ============================================================================
// VIEWPORT
// ============================================================================

void DisplayManager::setViewport(int32_t x, int32_t y, int32_t w, int32_t h) {
    m_viewX = max(0, x);
    m_viewY = max(0, y);
    m_viewW = min(w, DISPLAY_WIDTH - m_viewX);
    m_viewH = min(h, DISPLAY_HEIGHT - m_viewY);
}

void DisplayManager::resetViewport() {
    m_viewX = 0;
    m_viewY = 0;
    m_viewW = DISPLAY_WIDTH;
    m_viewH = DISPLAY_HEIGHT;
}

void DisplayManager::getViewport(int32_t& x, int32_t& y, int32_t& w, int32_t& h) const {
    x = m_viewX; y = m_viewY; w = m_viewW; h = m_viewH;
}

// ============================================================================
// БУФЕР
// ============================================================================

uint16_t* DisplayManager::getBuffer() {
    return m_framebuffer.buffer;
}

size_t DisplayManager::getBufferSize() const {
    return DISPLAY_WIDTH * DISPLAY_HEIGHT * sizeof(uint16_t);
}

void DisplayManager::flushRegion(int32_t x, int32_t y, int32_t w, int32_t h) {
    if (!m_framebuffer.buffer) return;

    // Обрезаем по экрану
    x = max(0, x);
    y = max(0, y);
    w = min(w, DISPLAY_WIDTH - x);
    h = min(h, DISPLAY_HEIGHT - y);

    // Отправляем область на дисплей
    m_tft.pushImage(x, y, w, h, m_framebuffer.buffer + y * m_framebuffer.stride + x);
}

void DisplayManager::setBufferPixel(int32_t x, int32_t y, uint16_t color) {
    if (!m_framebuffer.buffer || !isInBounds(x, y)) return;
    m_framebuffer.buffer[y * m_framebuffer.stride + x] = color;
}

uint16_t DisplayManager::getBufferPixel(int32_t x, int32_t y) const {
    if (!m_framebuffer.buffer || !isInBounds(x, y)) return 0;
    return m_framebuffer.buffer[y * m_framebuffer.stride + x];
}

// ============================================================================
// ЭФФЕКТЫ
// ============================================================================

void DisplayManager::fillNoise(uint16_t color1, uint16_t color2, uint8_t density) {
    if (!m_framebuffer.buffer) return;

    for (int32_t y = 0; y < DISPLAY_HEIGHT; y++) {
        for (int32_t x = 0; x < DISPLAY_WIDTH; x++) {
            if (random(100) < density) {
                setBufferPixel(x, y, random(2) ? color1 : color2);
            }
        }
    }
}

void DisplayManager::scrollHorizontal(int32_t offset) {
    if (!m_framebuffer.buffer) return;

    offset = offset % DISPLAY_WIDTH;
    if (offset == 0) return;

    // Временный буфер для строки
    uint16_t temp[DISPLAY_WIDTH];

    for (int32_t y = 0; y < DISPLAY_HEIGHT; y++) {
        int32_t row = y * m_framebuffer.stride;

        // Копируем строку во временный буфер
        memcpy(temp, m_framebuffer.buffer + row, DISPLAY_WIDTH * sizeof(uint16_t));

        // Сдвигаем
        if (offset > 0) {
            memcpy(m_framebuffer.buffer + row + offset, temp, (DISPLAY_WIDTH - offset) * sizeof(uint16_t));
            memcpy(m_framebuffer.buffer + row, temp + DISPLAY_WIDTH - offset, offset * sizeof(uint16_t));
        } else {
            int32_t absOffset = -offset;
            memcpy(m_framebuffer.buffer + row, temp + absOffset, (DISPLAY_WIDTH - absOffset) * sizeof(uint16_t));
            memcpy(m_framebuffer.buffer + row + DISPLAY_WIDTH - absOffset, temp, absOffset * sizeof(uint16_t));
        }
    }
}

void DisplayManager::scrollVertical(int32_t offset) {
    if (!m_framebuffer.buffer) return;

    offset = offset % DISPLAY_HEIGHT;
    if (offset == 0) return;

    size_t rowSize = DISPLAY_WIDTH * sizeof(uint16_t);
    size_t totalSize = DISPLAY_HEIGHT * rowSize;

    // Временный буфер для всего экрана
    uint16_t* temp = (uint16_t*)malloc(totalSize);
    if (!temp) return;

    memcpy(temp, m_framebuffer.buffer, totalSize);

    if (offset > 0) {
        memcpy(m_framebuffer.buffer + offset * DISPLAY_WIDTH, temp, (DISPLAY_HEIGHT - offset) * rowSize);
        memcpy(m_framebuffer.buffer, temp + (DISPLAY_HEIGHT - offset) * DISPLAY_WIDTH, offset * rowSize);
    } else {
        int32_t absOffset = -offset;
        memcpy(m_framebuffer.buffer, temp + absOffset * DISPLAY_WIDTH, (DISPLAY_HEIGHT - absOffset) * rowSize);
        memcpy(m_framebuffer.buffer + (DISPLAY_HEIGHT - absOffset) * DISPLAY_WIDTH, temp, absOffset * rowSize);
    }

    free(temp);
}

// ============================================================================
// ДОСТУП К TFT
// ============================================================================

TFT_eSPI& DisplayManager::getTFT() {
    return m_tft;
}

// ============================================================================
// ВСПОМОГАТЕЛЬНЫЕ
// ============================================================================

bool DisplayManager::isInBounds(int32_t x, int32_t y) const {
    return x >= 0 && x < DISPLAY_WIDTH && y >= 0 && y < DISPLAY_HEIGHT;
}

bool DisplayManager::isInViewport(int32_t x, int32_t y) const {
    return x >= m_viewX && x < m_viewX + m_viewW &&
           y >= m_viewY && y < m_viewY + m_viewH;
}