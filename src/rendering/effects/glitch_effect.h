/*
 * NEON REQUIEM - Glitch Effect
 * Digital distortion effect for GLITCH route and cyberpunk aesthetic
 * Implements scan line displacement, color channel offset, and block corruption
 */

#ifndef GLITCH_EFFECT_H
#define GLITCH_EFFECT_H

#include <Arduino.h>
#include "config/config.h"

// Glitch modes
enum GlitchMode : uint8_t {
    GLITCH_OFF = 0,
    GLITCH_SCAN_LINES = 1,     // Horizontal displacement bands
    GLITCH_CHANNEL_OFFSET = 2, // RGB channel separation
    GLITCH_BLOCK_CORRUPT = 3,  // Random block corruption
    GLITCH_FULL = 4            // All effects combined
};

// Glitch parameters
struct GlitchParams {
    uint8_t intensity;       // 0-255 overall intensity
    uint8_t scanLineFreq;    // Frequency of scan line displacement
    uint8_t channelOffset;   // Max channel offset in pixels
    uint8_t blockSize;       // Size of corruption blocks
    uint8_t blockFreq;       // Frequency of block corruption
    uint16_t duration;       // Effect duration in frames (0 = continuous)
};

class GlitchEffect {
public:
    // Singleton access
    static GlitchEffect& getInstance();
    
    // Control
    void begin();
    void reset();
    void trigger(GlitchMode mode, uint8_t intensity = 128, uint16_t duration = 30);
    void setMode(GlitchMode mode);
    void setIntensity(uint8_t intensity);
    void stop();
    bool isActive() const { return m_active; }
    GlitchMode getMode() const { return m_mode; }
    
    // Update and render
    void update();
    void render();
    
    // Configuration
    void setParams(const GlitchParams& params) { m_params = params; }
    const GlitchParams& getParams() const { return m_params; }

private:
    GlitchEffect() : m_active(false), m_mode(GLITCH_OFF), m_intensity(0), 
                     m_timer(0), m_frameCount(0) {}
    ~GlitchEffect() {}
    GlitchEffect(const GlitchEffect&) = delete;
    GlitchEffect& operator=(const GlitchEffect&) = delete;
    
    // Effect renderers
    void renderScanLines();
    void renderChannelOffset();
    void renderBlockCorrupt();
    
    bool m_active;
    GlitchMode m_mode;
    uint8_t m_intensity;
    GlitchParams m_params;
    uint16_t m_timer;
    uint16_t m_frameCount;
};

#endif // GLITCH_EFFECT_H