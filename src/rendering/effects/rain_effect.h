/*
 * NEON REQUIEM - Rain Effect
 * Atmospheric rain particle system for cyberpunk-noir aesthetic
 * Renders rain streaks with parallax depth layers
 */

#ifndef RAIN_EFFECT_H
#define RAIN_EFFECT_H

#include <Arduino.h>
#include "config/config.h"

#define MAX_RAIN_DROPS 120
#define RAIN_LAYERS 3

// Rain drop structure
struct RainDrop {
    int16_t x;
    int16_t y;
    int16_t speed;
    int8_t length;     // Streak length in pixels
    uint8_t opacity;   // 0-255
    uint8_t layer;     // 0=far, 1=mid, 2=near
    int8_t wind;       // Horizontal drift
    bool active;
};

class RainEffect {
public:
    // Singleton access
    static RainEffect& getInstance();
    
    // Control
    void begin();
    void reset();
    void start(uint8_t intensity = 80);  // 0-255 intensity
    void stop();
    void setIntensity(uint8_t intensity);
    uint8_t getIntensity() const { return m_intensity; }
    bool isActive() const { return m_active; }
    
    // Update and render
    void update();
    void render();
    
    // Configuration
    void setWind(int8_t wind) { m_wind = wind; }
    void setColor(uint16_t color) { m_rainColor = color; }
    void setLayerOpacity(uint8_t layer, uint8_t opacity);

private:
    RainEffect() : m_active(false), m_intensity(0), m_wind(0), 
                   m_rainColor(0x8410), m_timer(0) {}
    ~RainEffect() {}
    RainEffect(const RainEffect&) = delete;
    RainEffect& operator=(const RainEffect&) = delete;
    
    void spawnDrop(uint8_t layer);
    void updateDrop(RainDrop& drop);
    void renderDrop(const RainDrop& drop);
    
    RainDrop m_drops[MAX_RAIN_DROPS];
    bool m_active;
    uint8_t m_intensity;
    int8_t m_wind;
    uint16_t m_rainColor;
    uint8_t m_timer;
    
    // Layer configuration
    uint8_t m_layerCount[RAIN_LAYERS];     // Drops per layer
    uint8_t m_layerOpacity[RAIN_LAYERS];   // Opacity per layer
    int8_t m_layerSpeed[RAIN_LAYERS];      // Speed multiplier per layer
};

#endif // RAIN_EFFECT_H