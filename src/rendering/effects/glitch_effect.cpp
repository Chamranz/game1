/*
 * NEON REQUIEM - Glitch Effect Implementation
 * Digital distortion effect for GLITCH route and cyberpunk aesthetic
 */

#include "glitch_effect.h"
#include "../../core/display/display_manager.h"

// Singleton instance
GlitchEffect& GlitchEffect::getInstance() {
    static GlitchEffect instance;
    return instance;
}

void GlitchEffect::begin() {
    reset();
    LOG_INFO("GlitchEffect: Initialized");
}

void GlitchEffect::reset() {
    m_active = false;
    m_mode = GLITCH_OFF;
    m_intensity = 0;
    m_timer = 0;
    m_frameCount = 0;
    
    m_params.intensity = 128;
    m_params.scanLineFreq = 8;
    m_params.channelOffset = 4;
    m_params.blockSize = 8;
    m_params.blockFreq = 16;
    m_params.duration = 0;
}

void GlitchEffect::trigger(GlitchMode mode, uint8_t intensity, uint16_t duration) {
    m_mode = mode;
    m_intensity = constrain(intensity, 0, 255);
    m_params.duration = duration;
    m_params.intensity = m_intensity;
    m_timer = 0;
    m_active = true;
    
    LOG_VERBOSE("GlitchEffect: Triggered mode=%d intensity=%d duration=%d", 
                mode, intensity, duration);
}

void GlitchEffect::setMode(GlitchMode mode) {
    m_mode = mode;
    m_active = (mode != GLITCH_OFF);
}

void GlitchEffect::setIntensity(uint8_t intensity) {
    m_intensity = constrain(intensity, 0, 255);
    m_params.intensity = m_intensity;
}

void GlitchEffect::stop() {
    m_active = false;
    m_mode = GLITCH_OFF;
}

// ===== Update =====

void GlitchEffect::update() {
    if (!m_active) return;
    
    m_frameCount++;
    
    // Auto-stop if duration is set
    if (m_params.duration > 0 && m_frameCount >= m_params.duration) {
        stop();
        return;
    }
    
    m_timer++;
}

// ===== Render =====

void GlitchEffect::render() {
    if (!m_active || m_mode == GLITCH_OFF) return;
    
    switch (m_mode) {
        case GLITCH_SCAN_LINES:
            renderScanLines();
            break;
        case GLITCH_CHANNEL_OFFSET:
            renderChannelOffset();
            break;
        case GLITCH_BLOCK_CORRUPT:
            renderBlockCorrupt();
            break;
        case GLITCH_FULL:
            renderScanLines();
            renderChannelOffset();
            renderBlockCorrupt();
            break;
        default:
            break;
    }
}

void GlitchEffect::renderScanLines() {
    DisplayManager& display = DisplayManager::getInstance();
    
    // Apply horizontal displacement to scan lines
    if (m_timer % max(1, (int)(12 - m_intensity / 32)) == 0) {
        uint8_t numLines = map(m_intensity, 0, 255, 1, 8);
        
        for (int i = 0; i < numLines; i++) {
            int16_t lineY = random(DISPLAY_HEIGHT);
            int16_t lineH = random(1, 4);
            int16_t offset = random(-8, 8) * m_intensity / 128;
            
            // Shift a horizontal band
            for (int y = lineY; y < min(lineY + lineH, DISPLAY_HEIGHT); y++) {
                for (int x = 0; x < DISPLAY_WIDTH; x++) {
                    int16_t srcX = x + offset;
                    if (srcX >= 0 && srcX < DISPLAY_WIDTH) {
                        uint16_t pixel = display.readPixel(srcX, y);
                        display.drawPixel(x, y, pixel);
                    }
                }
            }
        }
    }
}

void GlitchEffect::renderChannelOffset() {
    DisplayManager& display = DisplayManager::getInstance();
    
    // Apply RGB channel separation
    if (m_timer % 3 == 0) {
        int16_t offset = random(-(int16_t)m_params.channelOffset, m_params.channelOffset + 1);
        offset = offset * m_intensity / 128;
        
        if (offset != 0) {
            int16_t stripY = random(DISPLAY_HEIGHT - 4);
            int16_t stripH = random(1, 3);
            
            for (int y = stripY; y < min(stripY + stripH, DISPLAY_HEIGHT); y++) {
                for (int x = 0; x < DISPLAY_WIDTH; x++) {
                    uint16_t pixel = display.readPixel(x, y);
                    
                    // Extract and shift red channel
                    uint8_t r = (pixel >> 11) & 0x1F;
                    uint8_t g = (pixel >> 5) & 0x3F;
                    uint8_t b = pixel & 0x1F;
                    
                    // Shift red channel left, blue channel right
                    int16_t rX = x - offset;
                    int16_t bX = x + offset;
                    
                    uint16_t newPixel = pixel;
                    
                    if (rX >= 0 && rX < DISPLAY_WIDTH) {
                        uint16_t rPixel = display.readPixel(rX, y);
                        uint8_t newR = (rPixel >> 11) & 0x1F;
                        newPixel = (newR << 11) | (g << 5) | b;
                    }
                    
                    if (bX >= 0 && bX < DISPLAY_WIDTH) {
                        uint16_t bPixel = display.readPixel(bX, y);
                        uint8_t newB = bPixel & 0x1F;
                        uint16_t currentR = (newPixel >> 11) & 0x1F;
                        uint16_t currentG = (newPixel >> 5) & 0x3F;
                        newPixel = (currentR << 11) | (currentG << 5) | newB;
                    }
                    
                    display.drawPixel(x, y, newPixel);
                }
            }
        }
    }
}

void GlitchEffect::renderBlockCorrupt() {
    DisplayManager& display = DisplayManager::getInstance();
    
    // Apply random block corruption
    if (m_timer % max(1, (int)(8 - m_intensity / 64)) == 0) {
        uint8_t numBlocks = map(m_intensity, 0, 255, 1, 6);
        
        for (int i = 0; i < numBlocks; i++) {
            int16_t bx = random(DISPLAY_WIDTH - m_params.blockSize);
            int16_t by = random(DISPLAY_HEIGHT - m_params.blockSize);
            uint8_t size = m_params.blockSize + random(-2, 3);
            
            // Fill block with random noise or shifted content
            for (int y = by; y < min(by + size, DISPLAY_HEIGHT); y++) {
                for (int x = bx; x < min(bx + size, DISPLAY_WIDTH); x++) {
                    if (random(100) < 60) {
                        // Random noise pixel
                        uint8_t r = random(32);
                        uint8_t g = random(64);
                        uint8_t b = random(32);
                        display.drawPixel(x, y, (r << 11) | (g << 5) | b);
                    } else {
                        // Shifted content from elsewhere
                        int16_t srcX = random(DISPLAY_WIDTH);
                        int16_t srcY = random(DISPLAY_HEIGHT);
                        display.drawPixel(x, y, display.readPixel(srcX, srcY));
                    }
                }
            }
        }
    }
}