/*
 * NEON REQUIEM - Dialog UI Implementation
 * Renders dialog boxes, choice menus, and speaker portraits
 */

#include "dialog_ui.h"
#include "../../core/display/display_manager.h"
#include "config/palette.h"

// Singleton instance
DialogUI& DialogUI::getInstance() {
    static DialogUI instance;
    return instance;
}

void DialogUI::begin() {
    reset();
    LOG_INFO("DialogUI: Initialized");
}

void DialogUI::reset() {
    // Nothing to reset
}

void DialogUI::drawDialogBox(uint16_t borderColor, uint16_t fillColor, uint16_t textColor) {
    DisplayManager& display = DisplayManager::getInstance();
    
    // Draw dialog box background
    display.fillRect(DIALOG_BOX_X, DIALOG_BOX_Y, DIALOG_BOX_W, DIALOG_BOX_H, fillColor);
    
    // Draw border
    display.drawRect(DIALOG_BOX_X, DIALOG_BOX_Y, DIALOG_BOX_W, DIALOG_BOX_H, borderColor);
    display.drawRect(DIALOG_BOX_X + 1, DIALOG_BOX_Y + 1, DIALOG_BOX_W - 2, DIALOG_BOX_H - 2, borderColor);
}

void DialogUI::drawSpeakerName(const char* name, uint16_t color) {
    DisplayManager& display = DisplayManager::getInstance();
    
    // Draw speaker name background
    display.fillRect(SPEAKER_NAME_X, SPEAKER_NAME_Y, SPEAKER_NAME_W, SPEAKER_NAME_H, Palette::BLACK);
    
    // Draw speaker name (simple rect as placeholder — no font renderer yet)
    display.drawRect(SPEAKER_NAME_X + 2, SPEAKER_NAME_Y + 1,
                     strlen(name) * 6, 8, color);
}

void DialogUI::drawDialogText(const char* text, uint16_t color, uint8_t charsToShow) {
    DisplayManager& display = DisplayManager::getInstance();
    
    // Clear text area
    display.fillRect(DIALOG_TEXT_X, DIALOG_TEXT_Y, DIALOG_TEXT_W, DIALOG_TEXT_H, Palette::BLACK);
    
    // Draw text character by character (simple pixel blocks as placeholder)
    uint8_t charCount = 0;
    uint8_t line = 0;
    uint8_t col = 0;
    
    for (const char* c = text; *c != '\0' && charCount < charsToShow; c++) {
        if (*c == '\n') {
            line++;
            col = 0;
            charCount++;
            continue;
        }
        
        if (col >= DIALOG_MAX_VISIBLE_CHARS) {
            line++;
            col = 0;
        }
        
        if (line >= DIALOG_MAX_VISIBLE_LINES) break;
        
        // Draw character as a small rect placeholder
        int16_t cx = DIALOG_TEXT_X + col * DIALOG_CHAR_W;
        int16_t cy = DIALOG_TEXT_Y + line * DIALOG_CHAR_H;
        display.fillRect(cx, cy, DIALOG_CHAR_W - 1, DIALOG_CHAR_H - 1, color);
        
        col++;
        charCount++;
    }
}

void DialogUI::drawContinueIndicator(uint8_t blinkPhase) {
    DisplayManager& display = DisplayManager::getInstance();
    
    if (blinkPhase % 2 == 0) {
        // Draw blinking triangle at bottom-right of dialog box
        int16_t triX = DIALOG_BOX_X + DIALOG_BOX_W - 12;
        int16_t triY = DIALOG_BOX_Y + DIALOG_BOX_H - 10;
        
        display.fillTriangle(triX, triY, triX + 6, triY, triX + 3, triY + 5, Palette::WHITE);
    }
}

void DialogUI::drawChoiceMenu(const char* choices[], uint8_t choiceCount, uint8_t selectedIndex) {
    DisplayManager& display = DisplayManager::getInstance();
    
    for (int i = 0; i < choiceCount; i++) {
        drawChoiceHighlight(i, i == selectedIndex);
        
        // Draw choice text as rect placeholder
        int16_t cy = CHOICE_START_Y + i * CHOICE_SPACING;
        display.fillRect(CHOICE_START_X + 12, cy, strlen(choices[i]) * 6, 8, Palette::WHITE);
    }
}

void DialogUI::drawChoiceHighlight(uint8_t index, bool selected) {
    DisplayManager& display = DisplayManager::getInstance();
    
    int16_t y = CHOICE_START_Y + index * CHOICE_SPACING;
    
    if (selected) {
        // Draw selection cursor
        display.fillTriangle(CHOICE_START_X - 4, y + 2,
                             CHOICE_START_X - 4, y + 8,
                             CHOICE_START_X, y + 5, Palette::CYAN);
    }
}

void DialogUI::drawPortrait(const uint16_t* pixels, uint8_t frame) {
    DisplayManager& display = DisplayManager::getInstance();
    
    if (!pixels) return;
    
    // Draw portrait pixels
    for (int y = 0; y < PORTRAIT_SIZE; y++) {
        for (int x = 0; x < PORTRAIT_SIZE; x++) {
            uint16_t pixel = pixels[y * PORTRAIT_SIZE + x];
            if (pixel != 0xF81F) { // Skip transparent
                display.drawPixel(PORTRAIT_X + x, PORTRAIT_Y + y, pixel);
            }
        }
    }
}

void DialogUI::drawPortraitBorder(uint16_t color) {
    DisplayManager& display = DisplayManager::getInstance();
    display.drawRect(PORTRAIT_X, PORTRAIT_Y, PORTRAIT_SIZE, PORTRAIT_SIZE, color);
}

void DialogUI::drawTypewriterText(const char* text, uint16_t color, uint8_t charIndex, uint8_t speed) {
    // Calculate how many characters to show based on index and speed
    uint8_t charsToShow = charIndex / speed;
    drawDialogText(text, color, charsToShow);
}

void DialogUI::drawShakeEffect(int8_t intensity) {
    // Shake is handled by offsetting the viewport in DisplayManager
    // This is a placeholder for the shake visual effect
    DisplayManager& display = DisplayManager::getInstance();
    display.setViewport(intensity, intensity, DISPLAY_WIDTH - intensity, DISPLAY_HEIGHT - intensity);
}

void DialogUI::drawGlitchLine(uint16_t y, uint16_t color) {
    DisplayManager& display = DisplayManager::getInstance();
    display.drawLine(DIALOG_BOX_X, y, DIALOG_BOX_X + DIALOG_BOX_W, y, color);
}