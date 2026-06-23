// ============================================================================
// NEON REQUIEM — Color Palette
// ============================================================================
// Полная цветовая палитра игры. Все цвета в формате RGB565 для TFT.
// Палитра динамически меняется в зависимости от выбранного маршрута:
//   RESONANCE — тёплые тона (cyan + amber)
//   ERASE     — агрессивные (red + deep bg)
//   GLITCH    — ломаные (magenta + green + glitch)
// ============================================================================

#ifndef NEON_REQUIEM_PALETTE_H
#define NEON_REQUIEM_PALETTE_H

#include <cstdint>
#include "config.h"

// ============================================================================
// ВСПОМОГАТЕЛЬНЫЕ МАКРОСЫ ДЛЯ RGB565
// ============================================================================
// Формат RGB565: 5 бит R, 6 бит G, 5 бит B
// ((R >> 3) << 11) | ((G >> 2) << 5) | (B >> 3)

/// Преобразование 8-битных R,G,B в RGB565
#define RGB565(r, g, b)  (((uint16_t)((r) >> 3) << 11) | \
                           ((uint16_t)((g) >> 2) << 5)  | \
                           ((uint16_t)((b) >> 3)))

/// Преобразование HEX (0xRRGGBB) в RGB565
#define HEX565(hex)      RGB565(((hex) >> 16) & 0xFF, \
                                 ((hex) >> 8)  & 0xFF, \
                                 (hex)         & 0xFF)

// ============================================================================
// БАЗОВАЯ ПАЛИТРА — общие цвета
// ============================================================================

namespace Palette {

/// Чистые цвета
constexpr uint16_t BLACK       = HEX565(0x000000);
constexpr uint16_t WHITE       = HEX565(0xFFFFFF);
constexpr uint16_t RED         = HEX565(0xFF0000);
constexpr uint16_t GREEN       = HEX565(0x00FF00);
constexpr uint16_t BLUE        = HEX565(0x0000FF);
constexpr uint16_t CYAN        = HEX565(0x00F0FF);
constexpr uint16_t MAGENTA     = HEX565(0xFF00AA);
constexpr uint16_t YELLOW      = HEX565(0xF0FF00);
constexpr uint16_t AMBER       = HEX565(0xFFAA00);

/// Прозрачный цвет (используется в спрайтах как key color)
constexpr uint16_t TRANSPARENT = HEX565(0xFF00FF);  // Ярко-розовый — chroma key

// ============================================================================
// АТМОСФЕРНЫЕ ЦВЕТА
// ============================================================================

/// Тёмный фон (небо, тени)
constexpr uint16_t DEEP_BG     = HEX565(0x0A0A12);

/// Цвет дождя
constexpr uint16_t RAIN        = HEX565(0x5C759B);

/// Туман
constexpr uint16_t FOG         = HEX565(0x8C9BB1);

/// Тёмно-синий (ночное небо)
constexpr uint16_t NIGHT_BLUE  = HEX565(0x0D0D2B);

/// Грязно-серый (бетон, асфальт)
constexpr uint16_t CONCRETE    = HEX565(0x3A3A4A);

/// Металлик (тёмный хром)
constexpr uint16_t METAL       = HEX565(0x5A5A6A);

// ============================================================================
// ЦВЕТА ПЕРСОНАЖЕЙ
// ============================================================================

/// KAI — плащ детектива
constexpr uint16_t KAI_COAT    = HEX565(0x314666);

/// KAI — свечение ядра в груди
constexpr uint16_t KAI_CORE    = HEX565(0x00F0FF);

/// ECHO — призрачный силуэт
constexpr uint16_t ECHO_GHOST  = HEX565(0x9772FF);

/// ECHO — глаза
constexpr uint16_t ECHO_EYES   = HEX565(0xCCAAFF);

/// RAZOR — кожаная куртка
constexpr uint16_t RAZOR_LEATHER = HEX565(0x2A1A0A);

/// LYRA — медицинский халат
constexpr uint16_t LYRA_COAT   = HEX565(0xE8E8F0);

/// ZERO — детский комбинезон
constexpr uint16_t ZERO_SUIT   = HEX565(0x40FF80);

/// CORTANA-9 — металлический корпус
constexpr uint16_t CORTANA_BODY = HEX565(0x8888AA);

/// SIREN — платье
constexpr uint16_t SIREN_DRESS = HEX565(0xFF0088);

/// DIRECTOR KANE — костюм
constexpr uint16_t KANE_SUIT   = HEX565(0x1A1A2A);

/// THE COLLECTOR — плащ
constexpr uint16_t COLLECTOR_ROBE = HEX565(0x2A0A2A);

// ============================================================================
// UI ЦВЕТА
// ============================================================================

/// HP Bar — полный
constexpr uint16_t HP_FULL     = HEX565(0x00FF00);

/// HP Bar — средний
constexpr uint16_t HP_MID      = HEX565(0xFFAA00);

/// HP Bar — низкий
constexpr uint16_t HP_LOW      = HEX565(0xFF0000);

/// Текст
constexpr uint16_t TEXT_WHITE  = HEX565(0xFFFFFF);
constexpr uint16_t TEXT_GRAY   = HEX565(0x888888);
constexpr uint16_t TEXT_DARK   = HEX565(0x444444);

/// Фон диалогового окна
constexpr uint16_t DIALOG_BG   = HEX565(0x0A0A1A);

/// Рамка диалогового окна
constexpr uint16_t DIALOG_BORDER = HEX565(0x00F0FF);

/// Имя говорящего (акцент)
constexpr uint16_t SPEAKER_NAME = HEX565(0xFFAA00);

/// Выбранный пункт меню
constexpr uint16_t MENU_SELECT = HEX565(0x00F0FF);

/// Невыбранный пункт меню
constexpr uint16_t MENU_NORMAL = HEX565(0x888888);

/// Кнопка MERCY (пощада)
constexpr uint16_t MERCY_COLOR = HEX565(0x00FF88);

/// Кнопка HACK (атака)
constexpr uint16_t HACK_COLOR  = HEX565(0xFF0044);

/// Кнопка SCAN (сканирование)
constexpr uint16_t SCAN_COLOR  = HEX565(0x4488FF);

/// Кнопка LINK (связь)
constexpr uint16_t LINK_COLOR  = HEX565(0xFFAA00);

// ============================================================================
// ПАЛИТРЫ МАРШРУТОВ
// ============================================================================
// Каждый маршрут меняет глобальную цветовую гамму.
// Структура содержит основные цвета для данного маршрута.

/// Палитра маршрута RESONANCE — тёплая, неоновая, надежда
constexpr uint16_t ROUTE_RESONANCE[] = {
    HEX565(0x0A0A1A),  // Фон (тёмно-синий)
    HEX565(0x00F0FF),  // Акцент 1 (циан)
    HEX565(0xFFAA00),  // Акцент 2 (амбер)
    HEX565(0x5C759B),  // Дождь
    HEX565(0x8C9BB1),  // Туман
    HEX565(0x314666),  // Тени
    HEX565(0xFFFFFF),  // Текст
    HEX565(0x00FF88)   // MERCY
};

/// Палитра маршрута ERASE — агрессивная, кровавая
constexpr uint16_t ROUTE_ERASE[] = {
    HEX565(0x0A0A0A),  // Фон (чёрный)
    HEX565(0xFF0000),  // Акцент 1 (красный)
    HEX565(0x880000),  // Акцент 2 (тёмно-красный)
    HEX565(0x442222),  // Дождь (кровавый)
    HEX565(0x664444),  // Туман
    HEX565(0x220000),  // Тени
    HEX565(0xFF4444),  // Текст (красный)
    HEX565(0xFF0044)   // MERCY (лже-пощада)
};

/// Палитра маршрута GLITCH — ломаная, искажённая
constexpr uint16_t ROUTE_GLITCH[] = {
    HEX565(0x0A000A),  // Фон (чёрно-фиолетовый)
    HEX565(0xFF00AA),  // Акцент 1 (маджента)
    HEX565(0x00FF44),  // Акцент 2 (зелёный)
    HEX565(0x440044),  // Дождь (фиолетовый)
    HEX565(0x664466),  // Туман
    HEX565(0x220022),  // Тени
    HEX565(0xCC88FF),  // Текст (фиолетовый)
    HEX565(0xFF44AA)   // MERCY (ломаный)
};

// ============================================================================
// ФУНКЦИИ РАБОТЫ С ЦВЕТОМ
// ============================================================================

/// Извлечение R, G, B из RGB565
inline uint8_t getRed565(uint16_t color)   { return (color >> 11) & 0x1F; }
inline uint8_t getGreen565(uint16_t color) { return (color >> 5)  & 0x3F; }
inline uint8_t getBlue565(uint16_t color)  { return color & 0x1F; }

/// Линейная интерполяция между двумя RGB565 цветами
/// @param a — первый цвет
/// @param b — второй цвет
/// @param t — коэффициент (0.0 = a, 1.0 = b)
inline uint16_t lerpColor(uint16_t a, uint16_t b, float t) {
    // Разворачиваем в 8-битные компоненты
    uint8_t ar = (a >> 11) & 0x1F;
    uint8_t ag = (a >> 5)  & 0x3F;
    uint8_t ab = a & 0x1F;

    uint8_t br = (b >> 11) & 0x1F;
    uint8_t bg = (b >> 5)  & 0x3F;
    uint8_t bb = b & 0x1F;

    // Интерполируем
    uint8_t resR = ar + (br - ar) * t;
    uint8_t resG = ag + (bg - ag) * t;
    uint8_t resB = ab + (bb - ab) * t;

    // Собираем обратно
    return (resR << 11) | (resG << 5) | b;
}

/// Затемнение цвета (0.0 = оригинал, 1.0 = чёрный)
inline uint16_t darkenColor(uint16_t color, float amount) {
    return lerpColor(color, Palette::BLACK, amount);
}

/// Осветление цвета (0.0 = оригинал, 1.0 = белый)
inline uint16_t lightenColor(uint16_t color, float amount) {
    return lerpColor(color, Palette::WHITE, amount);
}

/// Получить палитру для маршрута
/// @param route — маршрут (0=RESONANCE, 1=ERASE, 2=GLITCH)
/// @return указатель на массив из 8 цветов
inline const uint16_t* getRoutePalette(GameRoute route) {
    switch (route) {
        case GameRoute::RESONANCE: return ROUTE_RESONANCE;
        case GameRoute::ERASE:     return ROUTE_ERASE;
        case GameRoute::GLITCH:    return ROUTE_GLITCH;
        default:                   return ROUTE_RESONANCE;
    }
}

/// Индексы в палитре маршрута
enum RoutePaletteIndex : uint8_t {
    RPI_BG      = 0,  /// Фоновый цвет
    RPI_ACCENT1 = 1,  /// Акцент 1
    RPI_ACCENT2 = 2,  /// Акцент 2
    RPI_RAIN    = 3,  /// Цвет дождя
    RPI_FOG     = 4,  /// Цвет тумана
    RPI_SHADOW  = 5,  /// Цвет теней
    RPI_TEXT    = 6,  /// Цвет текста
    RPI_MERCY   = 7   /// Цвет пощады
};

} // namespace Palette

#endif // NEON_REQUIEM_PALETTE_H