/*
 * NEON REQUIEM - Rain Effect Implementation
 * Atmospheric rain particle system for cyberpunk-noir aesthetic
 */

#include "rain_effect.h"
#include "../../core/display/display_manager.h"

// Singleton instance
RainEffect& RainEffect::getInstance() {
    static RainEffect instance;
    return instance;
}

void RainEffect::begin() {
    reset();
    
    // Initialize layer configuration
    m_layerCount[0] = 30;   // Far layer: fewer, slower drops
    m_layerCount[1] = 40;   // Mid layer
    m_layerCount[2] = 50;   // Near layer: more, faster drops
    
    m_layerOpacity[0] = 80;   // Far: dimmer
    m_layerOpacity[1] = 160;  // Mid
    m_layerOpacity[2] = 220;  // Near: brighter
    
    m_layerSpeed[0] = 3;    // Far: slow
    m_layerSpeed[1] = 5;    // Mid
    m_layerSpeed[2] = 8;    // Near: fast
    
    LOG_INFO("RainEffect: Initialized");
}

void RainEffect::reset() {
    m_active = false;
    m_intensity = 0;
    m_wind = 0;
    m_rainColor = 0x8410;  // Gray-blue default
    m_timer = 0;
    
    for (int i = 0; i < MAX_RAIN_DROPS; i++) {
        m_drops[i].active = false;
    }
}

void RainEffect::start(uint8_t intensity) {
    m_intensity = constrain(intensity, 0, 255);
    m_active = true;
    
    // Spawn initial drops
    for (int l = 0; l < RAIN_LAYERS; l++) {
        for (int i = 0; i < m_layerCount[l]; i++) {
            spawnDrop(l);
        }
    }
    
    LOG_VERBOSE("RainEffect: Started (intensity=%d)", m_intensity);
}

void RainEffect::stop() {
    m_active = false;
    LOG_VERBOSE("RainEffect: Stopped");
}

void RainEffect::setIntensity(uint8_t intensity) {
    m_intensity = constrain(intensity, 0, 255);
}

void RainEffect::setLayerOpacity(uint8_t layer, uint8_t opacity) {
    if (layer < RAIN_LAYERS) {
        m_layerOpacity[layer] = opacity;
    }
}

// ===== Update and Render =====

void RainEffect::update() {
    if (!m_active) return;
    
    m_timer++;
    
    // Spawn new drops based on intensity
    uint8_t spawnRate = map(m_intensity, 0, 255, 0, 5);
    
    if (m_timer % max(1, (int)(6 - spawnRate)) == 0) {
        for (int l = 0; l < RAIN_LAYERS; l++) {
            if (random(100) < m_intensity / 3) {
                spawnDrop(l);
            }
        }
    }
    
    // Update existing drops
    for (int i = 0; i < MAX_RAIN_DROPS; i++) {
        if (m_drops[i].active) {
            updateDrop(m_drops[i]);
        }
    }
}

void RainEffect::render() {
    if (!m_active) return;
    
    DisplayManager& display = DisplayManager::getInstance();
    
    for (int i = 0; i < MAX_RAIN_DROPS; i++) {
        if (m_drops[i].active) {
            renderDrop(m_drops[i]);
        }
    }
}

// ===== Internal =====

void RainEffect::spawnDrop(uint8_t layer) {
    // Find inactive drop
    for (int i = 0; i < MAX_RAIN_DROPS; i++) {
        if (!m_drops[i].active) {
            RainDrop& drop = m_drops[i];
            drop.x = random(DISPLAY_WIDTH + 20) - 10;
            drop.y = random(-20, -5);
            drop.speed = m_layerSpeed[layer] + random(-1, 2);
            drop.length = 4 + layer * 3 + random(-1, 2);
            drop.opacity = m_layerOpacity[layer] + random(-20, 20);
            drop.layer = layer;
            drop.wind = m_wind + random(-1, 1);
            drop.active = true;
            return;
        }
    }
}

void RainEffect::updateDrop(RainDrop& drop) {
    drop.y += drop.speed;
    drop.x += drop.wind;
    
    // Reset if off screen
    if (drop.y > DISPLAY_HEIGHT + 10 || drop.x < -20 || drop.x > DISPLAY_WIDTH + 20) {
        drop.active = false;
    }
}

void RainEffect::renderDrop(const RainDrop& drop) {
    DisplayManager& display = DisplayManager::getInstance();
    
    // Draw rain streak
    int16_t endY = drop.y - drop.length;
    uint8_t alpha = drop.opacity;
    
    // Fade out at the top of the streak
    for (int i = 0; i < drop.length; i++) {
        int16_t py = drop.y - i;
        if (py < 0 || py >= DISPLAY_HEIGHT) continue;
        if (drop.x < 0 || drop.x >= DISPLAY_WIDTH) continue;
        
        uint8_t fadeAlpha = alpha * (drop.length - i) / drop.length;
        uint16_t color = m_rainColor;
        
        // Read background for alpha blend
        uint16_t bg = display.readPixel(drop.x, py);
        uint8_t r = (((color >> 11) & 0x1F) * fadeAlpha + ((bg >> 11) & 0x1F) * (255 - fadeAlpha)) / 255;
        uint8_t g = (((color >> 5) & 0x3F) * fadeAlpha + ((bg >> 5) & 0x3F) * (255 - fadeAlpha)) / 255;
        uint8_t b = ((color & 0x1F) * fadeAlpha + (bg & 0x1F) * (255 - fadeAlpha)) / 255;
        
        display.drawPixel(drop.x, py, (r << 11) | (g << 5) | b);
    }
}