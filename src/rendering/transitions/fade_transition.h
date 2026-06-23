/*
 * NEON REQUIEM - Fade Transition
 * Screen transition effects: fade in/out, wipe, radial, glitch
 * Used for scene changes, battle transitions, and cutscene transitions
 */

#ifndef FADE_TRANSITION_H
#define FADE_TRANSITION_H

#include <Arduino.h>
#include "config/config.h"
#include "config/palette.h"

// Transition types
enum TransitionType : uint8_t {
    TRANS_FADE_BLACK = 0,     // Fade to/from black
    TRANS_FADE_WHITE = 1,     // Fade to/from white
    TRANS_WIPE_LEFT = 2,      // Wipe from left to right
    TRANS_WIPE_RIGHT = 3,     // Wipe from right to left
    TRANS_WIPE_UP = 4,        // Wipe from bottom to top
    TRANS_WIPE_DOWN = 5,      // Wipe from top to bottom
    TRANS_RADIAL = 6,         // Radial reveal from center
    TRANS_GLITCH = 7,         // Glitch dissolve
    TRANS_PIXELATE = 8,       // Pixelation dissolve
    TRANS_NONE = 9            // Instant (no transition)
};

// Transition direction
enum TransitionDir : uint8_t {
    TRANS_IN = 0,   // Transition into scene (e.g., fade from black)
    TRANS_OUT = 1   // Transition out of scene (e.g., fade to black)
};

// Transition state
struct TransitionState {
    TransitionType type;
    TransitionDir direction;
    uint8_t progress;       // 0-255
    uint8_t speed;          // Progress per frame
    uint16_t color;         // Transition color (for fades)
    bool active;
    bool complete;
    void (*onComplete)();   // Callback when transition finishes
};

class FadeTransition {
public:
    // Singleton access
    static FadeTransition& getInstance();
    
    // Control
    void begin();
    void reset();
    
    // Start transition
    void start(TransitionType type, TransitionDir direction, uint8_t speed = 8,
               uint16_t color = Palette::BLACK, void (*callback)() = nullptr);
    void startFadeIn(uint8_t speed = 8, uint16_t color = Palette::BLACK);
    void startFadeOut(uint8_t speed = 8, uint16_t color = Palette::BLACK);
    
    // Update and render
    void update();
    void render();
    
    // State
    bool isActive() const { return m_state.active; }
    bool isComplete() const { return m_state.complete; }
    uint8_t getProgress() const { return m_state.progress; }
    TransitionType getType() const { return m_state.type; }
    
    // Utility
    void setSpeed(uint8_t speed) { m_state.speed = speed; }
    void setColor(uint16_t color) { m_state.color = color; }

private:
    FadeTransition() { reset(); }
    ~FadeTransition() {}
    FadeTransition(const FadeTransition&) = delete;
    FadeTransition& operator=(const FadeTransition&) = delete;
    
    void renderFade();
    void renderWipe();
    void renderRadial();
    void renderGlitch();
    void renderPixelate();
    
    TransitionState m_state;
    uint16_t m_timer;
};

#endif // FADE_TRANSITION_H