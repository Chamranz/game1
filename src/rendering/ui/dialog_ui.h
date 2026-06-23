/*
 * NEON REQUIEM - Dialog UI
 * Renders dialog boxes, choice menus, and speaker portraits
 * Route-specific styling for dialog presentation
 */

#ifndef DIALOG_UI_H
#define DIALOG_UI_H

#include <Arduino.h>
#include "config/config.h"

// Dialog box dimensions
#define DIALOG_BOX_X 8
#define DIALOG_BOX_Y 160
#define DIALOG_BOX_W (DISPLAY_WIDTH - 16)
#define DIALOG_BOX_H 72
#define DIALOG_TEXT_X (DIALOG_BOX_X + 8)
#define DIALOG_TEXT_Y (DIALOG_BOX_Y + 20)
#define DIALOG_TEXT_W (DIALOG_BOX_W - 16)
#define DIALOG_TEXT_H 40
#define DIALOG_CHAR_W 6
#define DIALOG_CHAR_H 8
#define DIALOG_MAX_VISIBLE_CHARS (DIALOG_TEXT_W / DIALOG_CHAR_W)
#define DIALOG_MAX_VISIBLE_LINES (DIALOG_TEXT_H / DIALOG_CHAR_H)

// Speaker name positions
#define SPEAKER_NAME_X (DIALOG_BOX_X + 8)
#define SPEAKER_NAME_Y (DIALOG_BOX_Y - 10)
#define SPEAKER_NAME_W 80
#define SPEAKER_NAME_H 12

// Choice menu positions
#define CHOICE_START_X (DIALOG_BOX_X + 16)
#define CHOICE_START_Y (DIALOG_BOX_Y + 8)
#define CHOICE_SPACING 14

// Portrait positions
#define PORTRAIT_X (DIALOG_BOX_X + DIALOG_BOX_W - 56)
#define PORTRAIT_Y (DIALOG_BOX_Y + 4)
#define PORTRAIT_SIZE 48

class DialogUI {
public:
    // Singleton access
    static DialogUI& getInstance();
    
    // Control
    void begin();
    void reset();
    
    // Dialog box rendering
    void drawDialogBox(uint16_t borderColor, uint16_t fillColor, uint16_t textColor);
    void drawSpeakerName(const char* name, uint16_t color);
    void drawDialogText(const char* text, uint16_t color, uint8_t charsToShow);
    void drawContinueIndicator(uint8_t blinkPhase);
    
    // Choice rendering
    void drawChoiceMenu(const char* choices[], uint8_t choiceCount, uint8_t selectedIndex);
    void drawChoiceHighlight(uint8_t index, bool selected);
    
    // Portrait rendering
    void drawPortrait(const uint16_t* pixels, uint8_t frame);
    void drawPortraitBorder(uint16_t color);
    
    // Typewriter effect
    void drawTypewriterText(const char* text, uint16_t color, uint8_t charIndex, uint8_t speed);
    
    // Special effects
    void drawShakeEffect(int8_t intensity);
    void drawGlitchLine(uint16_t y, uint16_t color);

private:
    DialogUI() {}
    ~DialogUI() {}
    DialogUI(const DialogUI&) = delete;
    DialogUI& operator=(const DialogUI&) = delete;
};

#endif // DIALOG_UI_H