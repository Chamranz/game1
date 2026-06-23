/*
 * NEON REQUIEM - CRT Effect Implementation
 * CRT monitor simulation for retro aesthetic
 */

#include "crt_effect.h"
#include "../../core/display/display_manager.h"

// Singleton instance
CRTEffect& CRTEffect::getInstance() {
    static CRTEffect instance;
    return instance;
}

void CRTEffect::begin() {
    reset();
    generateLUTs();
    LOG_INFO("CRTEffect: Initialized");
}

void CRTEffect::reset() {
    m_enabled = false;
    m_scanlineIntensity = 80;
    m_vignetteIntensity = 60;
    m_phosphorGlow = 40;
    m_curvature = 20;
}

void CRTEffect::generateLUTs() {
    // Pre-compute vignette lookup table
    int16_t centerX = DISPLAY_WIDTH / 2;
    int16_t centerY = DISPLAY_HEIGHT / 2;
    int16_t maxDist = (int16_t)sqrt(centerX * centerX + centerY * centerY);
    
    for (int y = 0; y < DISPLAY_HEIGHT; y++) {
        for (int x = 0; x < DISPLAY_WIDTH; x += 8) {
            uint8_t byteVal = 0;
            for (int b = 0; b < 8 && (x + b) < DISPLAY_WIDTH; b++) {
                int16_t dx = (x + b) - centerX;
                int16_t dy = y - centerY;
                int16_t dist = (int16_t)sqrt(dx * dx + dy * dy);
                uint8_t vignette = constrain(255 - (dist * 255 / maxDist), 0, 255);
                if (vignette < (255 - m_vignetteIntensity)) {
                    byteVal |= (1 << b);
                }
            }
            m_vignetteLUT[y][x / 8] = byteVal;
        }
    }
}

void CRTEffect::render() {
    if (!m_enabled) return;
    
    renderScanlines();
    renderVignette();
}

void CRTEffect::renderScanlines() {
    DisplayManager& display = DisplayManager::getInstance();
    
    // Draw scanlines on every other row
    for (int y = 0; y < DISPLAY_HEIGHT; y += 2) {
        for (int x = 0; x < DISPLAY_WIDTH; x++) {
            uint16_t pixel = display.readPixel(x, y);
            
            // Darken scanline rows
            uint8_t r = ((pixel >> 11) & 0x1F);
            uint8_t g = ((pixel >> 5) & 0x3F);
            uint8_t b = (pixel & 0x1F);
            
            r = r * (255 - m_scanlineIntensity) / 255;
            g = g * (255 - m_scanlineIntensity) / 255;
            b = b * (255 - m_scanlineIntensity) / 255;
            
            display.drawPixel(x, y, (r << 11) | (g << 5) | b);
        }
    }
}

void CRTEffect::renderVignette() {
    DisplayManager& display = DisplayManager::getInstance();
    
    // Apply vignette darkening at screen edges
    int16_t centerX = DISPLAY_WIDTH / 2;
    int16_t centerY = DISPLAY_HEIGHT / 2;
    int16_t maxDist = (int16_t)sqrt(centerX * centerX + centerY * centerY);
    
    for (int y = 0; y < DISPLAY_HEIGHT; y++) {
        for (int x = 0; x < DISPLAY_WIDTH; x++) {
            // Check LUT
            if (m_vignetteLUT[y][x / 8] & (1 << (x % 8))) {
                int16_t dx = x - centerX;
                int16_t dy = y - centerY;
                int16_t dist = (int16_t)sqrt(dx * dx + dy * dy);
                uint8_t darken = constrain((dist * m_vignetteIntensity / maxDist), 0, 255);
                
                uint16_t pixel = display.readPixel(x, y);
                uint8_t r = ((pixel >> 11) & 0x1F) * (255 - darken) / 255;
                uint8_t g = ((pixel >> 5) & 0x3F) * (255 - darken) / 255;
                uint8_t b = (pixel & 0x1F) * (255 - darken) / 255;
                
                display.drawPixel(x, y, (r << 11) | (g << 5) | b);
            }
        }
    }
}