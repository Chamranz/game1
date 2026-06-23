/*
 * NEON REQUIEM - CRT Effect
 * CRT monitor simulation for retro aesthetic
 * Implements scanlines, vignette, phosphor glow, and screen curvature
 */

#ifndef CRT_EFFECT_H
#define CRT_EFFECT_H

#include <Arduino.h>
#include "config/config.h"

class CRTEffect {
public:
    // Singleton access
    static CRTEffect& getInstance();
    
    // Control
    void begin();
    void reset();
    void setEnabled(bool enabled) { m_enabled = enabled; }
    bool isEnabled() const { return m_enabled; }
    
    // Configuration
    void setScanlineIntensity(uint8_t intensity) { m_scanlineIntensity = intensity; }
    void setVignetteIntensity(uint8_t intensity) { m_vignetteIntensity = intensity; }
    void setPhosphorGlow(uint8_t glow) { m_phosphorGlow = glow; }
    void setCurvature(uint8_t curvature) { m_curvature = curvature; }
    
    // Render
    void render();
    
    // Pre-computed lookup tables
    void generateLUTs();

private:
    CRTEffect() : m_enabled(false), m_scanlineIntensity(80), 
                  m_vignetteIntensity(60), m_phosphorGlow(40), m_curvature(20) {}
    ~CRTEffect() {}
    CRTEffect(const CRTEffect&) = delete;
    CRTEffect& operator=(const CRTEffect&) = delete;
    
    void renderScanlines();
    void renderVignette();
    
    bool m_enabled;
    uint8_t m_scanlineIntensity;
    uint8_t m_vignetteIntensity;
    uint8_t m_phosphorGlow;
    uint8_t m_curvature;
    
    // Pre-computed vignette LUT
    uint8_t m_vignetteLUT[DISPLAY_HEIGHT][DISPLAY_WIDTH / 8]; // Compressed
};

#endif // CRT_EFFECT_H