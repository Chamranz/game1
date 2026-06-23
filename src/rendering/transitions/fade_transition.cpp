/*
 * NEON REQUIEM - Fade Transition Implementation
 * Screen transition effects: fade in/out, wipe, radial, glitch
 */

#include "fade_transition.h"
#include "../../core/display/display_manager.h"
#include "config/palette.h"

// Singleton instance
FadeTransition& FadeTransition::getInstance() {
    static FadeTransition instance;
    return instance;
}

void FadeTransition::begin() {
    reset();
    LOG_INFO("FadeTransition: Initialized");
}

void FadeTransition::reset() {
    m_state.type = TRANS_NONE;
    m_state.direction = TRANS_IN;
    m_state.progress = 0;
    m_state.speed = 8;
    m_state.color = Palette::BLACK;
    m_state.active = false;
    m_state.complete = false;
    m_state.onComplete = nullptr;
    m_timer = 0;
}

void FadeTransition::start(TransitionType type, TransitionDir direction, uint8_t speed,
                            uint16_t color, void (*callback)()) {
    m_state.type = type;
    m_state.direction = direction;
    m_state.speed = speed;
    m_state.color = color;
    m_state.onComplete = callback;
    m_state.progress = 0;
    m_state.active = true;
    m_state.complete = false;
    m_timer = 0;
    
    // For fade-in, start fully covered
    if (direction == TRANS_IN) {
        m_state.progress = 0;
    } else {
        m_state.progress = 0;
    }
}

void FadeTransition::startFadeIn(uint8_t speed, uint16_t color) {
    start(TRANS_FADE_BLACK, TRANS_IN, speed, color);
}

void FadeTransition::startFadeOut(uint8_t speed, uint16_t color) {
    start(TRANS_FADE_BLACK, TRANS_OUT, speed, color);
}

void FadeTransition::update() {
    if (!m_state.active || m_state.complete) return;
    
    m_timer++;
    
    // Advance progress
    m_state.progress += m_state.speed;
    
    if (m_state.progress >= 255) {
        m_state.progress = 255;
        m_state.complete = true;
        m_state.active = false;
        
        // Call completion callback
        if (m_state.onComplete) {
            m_state.onComplete();
        }
    }
}

void FadeTransition::render() {
    if (!m_state.active && !m_state.complete) return;
    if (m_state.type == TRANS_NONE) return;
    
    switch (m_state.type) {
        case TRANS_FADE_BLACK:
        case TRANS_FADE_WHITE:
            renderFade();
            break;
        case TRANS_WIPE_LEFT:
        case TRANS_WIPE_RIGHT:
        case TRANS_WIPE_UP:
        case TRANS_WIPE_DOWN:
            renderWipe();
            break;
        case TRANS_RADIAL:
            renderRadial();
            break;
        case TRANS_GLITCH:
            renderGlitch();
            break;
        case TRANS_PIXELATE:
            renderPixelate();
            break;
        default:
            break;
    }
}

void FadeTransition::renderFade() {
    DisplayManager& display = DisplayManager::getInstance();
    
    uint8_t alpha;
    if (m_state.direction == TRANS_IN) {
        // Fade from color to clear (progress 255 -> 0)
        alpha = 255 - m_state.progress;
    } else {
        // Fade from clear to color (progress 0 -> 255)
        alpha = m_state.progress;
    }
    
    // Draw overlay with alpha
    if (alpha > 0) {
        // Simple alpha: draw semi-transparent overlay
        for (int y = 0; y < DISPLAY_HEIGHT; y += 2) {
            for (int x = 0; x < DISPLAY_WIDTH; x += 2) {
                uint16_t bg = display.readPixel(x, y);
                uint8_t r = (((m_state.color >> 11) & 0x1F) * alpha + ((bg >> 11) & 0x1F) * (255 - alpha)) / 255;
                uint8_t g = (((m_state.color >> 5) & 0x3F) * alpha + ((bg >> 5) & 0x3F) * (255 - alpha)) / 255;
                uint8_t b = ((m_state.color & 0x1F) * alpha + (bg & 0x1F) * (255 - alpha)) / 255;
                display.drawPixel(x, y, (r << 11) | (g << 5) | b);
                display.drawPixel(x + 1, y, (r << 11) | (g << 5) | b);
                display.drawPixel(x, y + 1, (r << 11) | (g << 5) | b);
                display.drawPixel(x + 1, y + 1, (r << 11) | (g << 5) | b);
            }
        }
    }
}

void FadeTransition::renderWipe() {
    DisplayManager& display = DisplayManager::getInstance();
    
    uint8_t progress = m_state.progress;
    if (m_state.direction == TRANS_IN) {
        progress = 255 - progress;
    }
    
    int16_t wipePos = (int16_t)progress * DISPLAY_WIDTH / 255;
    
    switch (m_state.type) {
        case TRANS_WIPE_LEFT:
            display.fillRect(0, 0, wipePos, DISPLAY_HEIGHT, m_state.color);
            break;
        case TRANS_WIPE_RIGHT:
            display.fillRect(DISPLAY_WIDTH - wipePos, 0, wipePos, DISPLAY_HEIGHT, m_state.color);
            break;
        case TRANS_WIPE_UP:
            display.fillRect(0, 0, DISPLAY_WIDTH, wipePos, m_state.color);
            break;
        case TRANS_WIPE_DOWN:
            display.fillRect(0, DISPLAY_HEIGHT - wipePos, DISPLAY_WIDTH, wipePos, m_state.color);
            break;
        default:
            break;
    }
}

void FadeTransition::renderRadial() {
    DisplayManager& display = DisplayManager::getInstance();
    
    uint8_t progress = m_state.progress;
    if (m_state.direction == TRANS_IN) {
        progress = 255 - progress;
    }
    
    int16_t centerX = DISPLAY_WIDTH / 2;
    int16_t centerY = DISPLAY_HEIGHT / 2;
    int16_t maxRadius = (int16_t)sqrt(centerX * centerX + centerY * centerY);
    int16_t radius = (int16_t)progress * maxRadius / 255;
    
    // Draw radial circle
    for (int y = 0; y < DISPLAY_HEIGHT; y++) {
        for (int x = 0; x < DISPLAY_WIDTH; x++) {
            int16_t dx = x - centerX;
            int16_t dy = y - centerY;
            int16_t dist = (int16_t)sqrt(dx * dx + dy * dy);
            if (dist > radius) {
                display.drawPixel(x, y, m_state.color);
            }
        }
    }
}

void FadeTransition::renderGlitch() {
    DisplayManager& display = DisplayManager::getInstance();
    
    uint8_t progress = m_state.progress;
    if (m_state.direction == TRANS_IN) {
        progress = 255 - progress;
    }
    
    // Glitch dissolve: random horizontal strips
    uint8_t stripCount = map(progress, 0, 255, 0, 40);
    
    for (int i = 0; i < stripCount; i++) {
        int16_t y = random(DISPLAY_HEIGHT);
        int16_t h = random(1, 6);
        display.fillRect(0, y, DISPLAY_WIDTH, h, m_state.color);
    }
}

void FadeTransition::renderPixelate() {
    DisplayManager& display = DisplayManager::getInstance();
    
    uint8_t progress = m_state.progress;
    if (m_state.direction == TRANS_IN) {
        progress = 255 - progress;
    }
    
    // Pixelation: draw large blocks
    uint8_t blockSize = map(progress, 0, 255, 1, 16);
    
    for (int y = 0; y < DISPLAY_HEIGHT; y += blockSize) {
        for (int x = 0; x < DISPLAY_WIDTH; x += blockSize) {
            uint16_t pixel = display.readPixel(x, y);
            display.fillRect(x, y, blockSize, blockSize, pixel);
        }
    }
}