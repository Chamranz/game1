// ============================================================================
// NEON REQUIEM — Dialog Box (Header)
// ============================================================================
// Компонент отрисовки диалогового окна. Отвечает за:
// - Рендеринг окна с рамкой
// - Typewriter-эффект текста
// - Портрет говорящего (цветной квадрат)
// - Список вариантов выбора
// - Индикатор продолжения
// ============================================================================

#ifndef NEON_REQUIEM_DIALOG_BOX_H
#define NEON_REQUIEM_DIALOG_BOX_H

#include <cstdint>
#include <Arduino.h>
#include "config/config.h"
#include "config/palette.h"

/// Размеры диалогового окна
constexpr int DIALOG_BOX_X      = 4;
constexpr int DIALOG_BOX_Y      = DISPLAY_HEIGHT - 84;
constexpr int DIALOG_BOX_W      = DISPLAY_WIDTH - 8;
constexpr int DIALOG_BOX_H      = 80;
constexpr int DIALOG_TEXT_X     = DIALOG_BOX_X + 8;
constexpr int DIALOG_TEXT_Y     = DIALOG_BOX_Y + 20;
constexpr int DIALOG_NAME_X     = DIALOG_BOX_X + 8;
constexpr int DIALOG_NAME_Y     = DIALOG_BOX_Y + 4;
constexpr int DIALOG_CHOICE_X   = DIALOG_BOX_X + 16;
constexpr int DIALOG_CHOICE_Y   = DIALOG_BOX_Y + 24;
constexpr int DIALOG_CHOICE_GAP = 14;

/// Класс для отрисовки диалогового окна
class DialogBox {
public:
    DialogBox();

    /// Отрисовать фон окна
    void drawBackground(uint16_t borderColor = Palette::DIALOG_BORDER);

    /// Отрисовать имя говорящего
    void drawSpeakerName(const char* name, uint16_t color);

    /// Отрисовать текст с typewriter-эффектом
    /// @param text — полный текст
    /// @param visibleChars — сколько символов показать
    void drawText(const char* text, uint16_t visibleChars);

    /// Отрисовать вариант выбора
    /// @param index — номер варианта (0-3)
    /// @param text — текст варианта
    /// @param selected — выбран ли
    void drawChoice(int index, const char* text, bool selected);

    /// Отрисовать индикатор продолжения (мигающий треугольник)
    void drawContinueIndicator();

    /// Отрисовать портрет говорящего
    /// @param color — цвет портрета
    void drawPortrait(uint16_t color);

    /// Очистить область текста
    void clearTextArea();

private:
    /// Внутренний буфер для typewriter
    char m_lineBuffer[DIALOG_MAX_LINE_LENGTH];
};

#endif // NEON_REQUIEM_DIALOG_BOX_H