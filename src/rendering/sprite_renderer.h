/*
 * NEON REQUIEM - Sprite Renderer
 * Handles sprite loading from SD, caching in PSRAM, and rendering
 * Supports sprite sheets, flipping, tinting, and alpha blending
 */

#ifndef SPRITE_RENDERER_H
#define SPRITE_RENDERER_H

#include <Arduino.h>
#include "config/config.h"

// Sprite constants
#define MAX_CACHED_SPRITES 32
#define MAX_SPRITE_WIDTH 64
#define MAX_SPRITE_HEIGHT 64
#define SPRITE_CACHE_SIZE (320 * 1024)  // 320KB for sprite cache

// Sprite flags
#define SPRITE_FLIP_H 0x01
#define SPRITE_FLIP_V 0x02
#define SPRITE_ALPHA  0x04
#define SPRITE_TINTED 0x08

// Sprite data structure (in PSRAM)
struct SpriteData {
    char id[20];              // Sprite identifier
    uint16_t width;           // Sprite width in pixels
    uint16_t height;          // Sprite height in pixels
    uint16_t frames;          // Number of animation frames
    uint16_t frameWidth;      // Width of single frame
    uint16_t frameHeight;     // Height of single frame
    uint16_t* pixels;         // Pointer to pixel data in PSRAM
    bool loaded;              // Is sprite loaded in cache?
    uint32_t lastAccess;      // Last access time for cache eviction
};

// Sprite rendering request
struct SpriteRenderRequest {
    const char* spriteId;     // Sprite to render
    int16_t x;                // Screen X position
    int16_t y;                // Screen Y position
    uint8_t frame;            // Animation frame index
    uint8_t flags;            // Render flags (flip, alpha, tint)
    uint16_t tintColor;       // Tint color (RGB565), 0=no tint
    uint8_t alpha;            // Alpha value (0-255), 255=opaque
    uint8_t scale;            // Scale factor (1-4), 1=normal
};

class SpriteRenderer {
public:
    // Singleton access
    static SpriteRenderer& getInstance();
    
    // Initialization
    void begin();
    void reset();
    
    // Sprite loading and caching
    bool loadSprite(const char* spriteId, const char* filePath);
    bool loadSpriteSheet(const char* spriteId, const char* filePath, 
                         uint16_t frameWidth, uint16_t frameHeight, uint16_t frames);
    void unloadSprite(const char* spriteId);
    void unloadAll();
    bool isSpriteLoaded(const char* spriteId) const;
    SpriteData* getSprite(const char* spriteId);
    
    // Rendering
    void drawSprite(const SpriteRenderRequest& request);
    void drawSpriteFrame(const char* spriteId, int16_t x, int16_t y, uint8_t frame);
    void drawSpriteScaled(const char* spriteId, int16_t x, int16_t y, uint8_t frame, uint8_t scale);
    void drawSpriteTinted(const char* spriteId, int16_t x, int16_t y, uint8_t frame, uint16_t tintColor);
    void drawSpriteAlpha(const char* spriteId, int16_t x, int16_t y, uint8_t frame, uint8_t alpha);
    void drawSpriteFlipped(const char* spriteId, int16_t x, int16_t y, uint8_t frame, bool flipH, bool flipV);
    
    // Batch rendering (for performance)
    void beginBatch();
    void submitRequest(const SpriteRenderRequest& request);
    void endBatch();  // Renders all batched sprites
    
    // Cache management
    void evictOldest();  // Remove least recently used sprite
    uint16_t getCacheUsage() const;
    uint8_t getCachedCount() const { return m_cachedCount; }
    
    // Utility
    uint16_t getSpriteWidth(const char* spriteId) const;
    uint16_t getSpriteHeight(const char* spriteId) const;
    uint8_t getSpriteFrameCount(const char* spriteId) const;

private:
    SpriteRenderer() : m_cachedCount(0), m_batchCount(0), m_cacheUsage(0) {}
    ~SpriteRenderer() {}
    SpriteRenderer(const SpriteRenderer&) = delete;
    SpriteRenderer& operator=(const SpriteRenderer&) = delete;
    
    // Internal rendering
    void renderSprite(const SpriteData* sprite, int16_t x, int16_t y, 
                      uint8_t frame, uint8_t flags, uint16_t tintColor, 
                      uint8_t alpha, uint8_t scale);
    
    // Find sprite in cache
    int8_t findSpriteIndex(const char* spriteId) const;
    
    SpriteData m_cache[MAX_CACHED_SPRITES];
    uint8_t m_cachedCount;
    uint16_t m_cacheUsage;  // Current cache usage in bytes
    
    // Batch buffer
    SpriteRenderRequest m_batchBuffer[64];
    uint8_t m_batchCount;
    bool m_batching;
};

#endif // SPRITE_RENDERER_H