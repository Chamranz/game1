// ============================================================================
// NEON REQUIEM — Dialog Box (Implementation)
// ============================================================================
// Реализация отрисовки диалогового окна. Использует примитивы DisplayManager
// для рендеринга текста, рамок и индикаторов.
// ============================================================================

#include "dialog_box.h"
#include "../../core/display/display_manager.h"

DialogBox::DialogBox() {
    m_lineBuffer[0] = '\0';
}

void DialogBox::drawBackground(uint16_t borderColor) {
    DisplayManager& display = DisplayManager::getInstance();

    // Фон
    display.fillRect(DIALOG_BOX_X, DIALOG_BOX_Y,
                     DIALOG_BOX_W, DIALOG_BOX_H, Palette::DIALOG_BG);

    // Рамка (двойная линия)
    display.drawRect(DIALOG_BOX_X, DIALOG_BOX_Y,
                     DIALOG_BOX_W, DIALOG_BOX_H, borderColor);
    display.drawRect(DIALOG_BOX_X + 1, DIALOG_BOX_Y + 1,
                     DIALOG_BOX_W - 2, DIALOG_BOX_H - 2,
                     Palette::darkenColor(borderColor, 0.5f));
}

void DialogBox::drawSpeakerName(const char* name, uint16_t color) {
    DisplayManager& display = DisplayManager::getInstance();

    // Фон под именем
    display.fillRect(DIALOG_NAME_X - 2, DIALOG_NAME_Y - 2,
                     strlen(name) * 8 + 4, 12, Palette::DIALOG_BG);

    // TODO: отрисовка текста через шрифтовую систему
    // Пока — цветной прямоугольник как placeholder
    display.fillRect(DIALOG_NAME_X, DIALOG_NAME_Y,
                     strlen(name) * 8, 8, color);
}

void DialogBox::drawText(const char* text, uint16_t visibleChars) {
    DisplayManager& display = DisplayManager::getInstance();

    // Очищаем область текста
    clearTextArea();

    // Копируем видимую часть текста
    uint16_t len = min(visibleChars, (uint16_t)strlen(text));
    strncpy(m_lineBuffer, text, len);
    m_lineBuffer[len] = '\0';

    // TODO: отрисовка текста через шрифтовую систему
    // Пока — цветная полоса как placeholder
    if (len > 0) {
        display.fillRect(DIALOG_TEXT_X, DIALOG_TEXT_Y,
                         len * 6, 8, Palette::TEXT_WHITE);
    }
}

void DialogBox::drawChoice(int index, const char* text, bool selected) {
    DisplayManager& display = DisplayManager::getInstance();

    int y = DIALOG_CHOICE_Y + index * DIALOG_CHOICE_GAP;
    uint16_t color = selected ? Palette::MENU_SELECT : Palette::MENU_NORMAL;

    // Маркер выбора
    if (selected) {
        display.fillTriangle(
            DIALOG_CHOICE_X - 10, y + 4,
            DIALOG_CHOICE_X - 4,  y,
            DIALOG_CHOICE_X - 4,  y + 8,
            Palette::MENU_SELECT
        );
    }

    // TODO: отрисовка текста варианта
    display.fillRect(DIALOG_CHOICE_X, y, strlen(text) * 6, 8, color);
}

void DialogBox::drawContinueIndicator() {
    DisplayManager& display = DisplayManager::getInstance();

    static uint32_t blinkTimer = 0;
    blinkTimer += FRAME_TIME_MS;

    if ((blinkTimer / 500) % 2 == 0) {
        display.fillTriangle(
            DIALOG_BOX_X + DIALOG_BOX_W - 16,
            DIALOG_BOX_Y + DIALOG_BOX_H - 8,
            DIALOG_BOX_X + DIALOG_BOX_W - 8,
            DIALOG_BOX_Y + DIALOG_BOX_H - 4,
            DIALOG_BOX_X + DIALOG_BOX_W - 16,
            DIALOG_BOX_Y + DIALOG_BOX_H,
            Palette::TEXT_WHITE
        );
    }
}

void DialogBox::drawPortrait(uint16_t color) {
    DisplayManager& display = DisplayManager::getInstance();

    // Портрет — цветной квадрат 16x16 слева от имени
    constexpr int PORTRAIT_X = DIALOG_BOX_X + 4;
    constexpr int PORTRAIT_Y = DIALOG_BOX_Y + 4;
    constexpr int PORTRAIT_S = 16;

    display.fillRect(PORTRAIT_X, PORTRAIT_Y, PORTRAIT_S, PORTRAIT_S, color);
    display.drawRect(PORTRAIT_X, PORTRAIT_Y, PORTRAIT_S, PORTRAIT_S, Palette::WHITE);
}

void DialogBox::clearTextArea() {
    DisplayManager& display = DisplayManager::getInstance();

    display.fillRect(
        DIALOG_TEXT_X, DIALOG_TEXT_Y,
        DIALOG_BOX_W - 16, DIALOG_BOX_H - 28,
        Palette::DIALOG_BG
    );
}