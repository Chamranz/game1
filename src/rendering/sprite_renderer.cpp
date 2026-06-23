/*
 * NEON REQUIEM - Sprite Renderer Implementation
 * Handles sprite loading from SD, caching in PSRAM, and rendering
 */

#include "sprite_renderer.h"
#include "../core/display/display_manager.h"
#include "../core/storage/sd_manager.h"

// Singleton instance
SpriteRenderer& SpriteRenderer::getInstance() {
    static SpriteRenderer instance;
    return instance;
}

void SpriteRenderer::begin() {
    reset();
    m_batching = false;
    LOG_INFO("SpriteRenderer: Initialized (%d sprite cache, %dKB)", 
             MAX_CACHED_SPRITES, SPRITE_CACHE_SIZE / 1024);
}

void SpriteRenderer::reset() {
    // Free all PSRAM allocations
    for (int i = 0; i < m_cachedCount; i++) {
        if (m_cache[i].pixels) {
            free(m_cache[i].pixels);
            m_cache[i].pixels = nullptr;
        }
    }
    m_cachedCount = 0;
    m_cacheUsage = 0;
    m_batchCount = 0;
}

// ===== Sprite Loading =====

bool SpriteRenderer::loadSprite(const char* spriteId, const char* filePath) {
    SDManager& sd = SDManager::getInstance();
    
    // Check if already loaded
    if (isSpriteLoaded(spriteId)) {
        LOG_VERBOSE("SpriteRenderer: '%s' already cached", spriteId);
        return true;
    }
    
    // Check if cache is full
    if (m_cachedCount >= MAX_CACHED_SPRITES) {
        evictOldest();
    }
    
    // Read sprite file using SD manager
    size_t fileSize = sd.getFileSize(filePath);
    if (fileSize == 0) {
        LOG_ERROR("SpriteRenderer: Cannot load '%s' - file not found", filePath);
        return false;
    }
    
    // Read file data
    uint8_t* fileBuffer = sd.readFile(filePath, fileSize);
    if (!fileBuffer) {
        LOG_ERROR("SpriteRenderer: Failed to read file '%s'", filePath);
        return false;
    }
    
    // Parse header
    uint16_t width = fileBuffer[0] | (fileBuffer[1] << 8);
    uint16_t height = fileBuffer[2] | (fileBuffer[3] << 8);
    uint16_t frames = fileBuffer[4] | (fileBuffer[5] << 8);
    uint16_t frameWidth = fileBuffer[6] | (fileBuffer[7] << 8);
    uint16_t frameHeight = fileBuffer[8] | (fileBuffer[9] << 8);
    
    // Validate dimensions
    if (width > MAX_SPRITE_WIDTH || height > MAX_SPRITE_HEIGHT || frames == 0) {
        free(fileBuffer);
        LOG_ERROR("SpriteRenderer: Invalid sprite dimensions (%dx%d, %d frames)",
                  width, height, frames);
        return false;
    }
    
    // Calculate pixel data size
    uint16_t pixelCount = width * height;
    uint16_t pixelDataSize = pixelCount * sizeof(uint16_t);
    
    // Check cache space
    if (m_cacheUsage + pixelDataSize > SPRITE_CACHE_SIZE) {
        evictOldest();
    }
    
    // Allocate PSRAM for pixel data
    uint16_t* pixels = (uint16_t*)ps_malloc(pixelDataSize);
    if (!pixels) {
        free(fileBuffer);
        LOG_ERROR("SpriteRenderer: Failed to allocate pixel data for '%s' (%d bytes)",
                  spriteId, pixelDataSize);
        return false;
    }
    
    // Copy pixel data (after 10-byte header)
    memcpy(pixels, fileBuffer + 10, pixelDataSize);
    free(fileBuffer);
    
    // Store in cache
    SpriteData& sprite = m_cache[m_cachedCount];
    strncpy(sprite.id, spriteId, sizeof(sprite.id) - 1);
    sprite.id[sizeof(sprite.id) - 1] = '\0';
    sprite.width = width;
    sprite.height = height;
    sprite.frames = frames;
    sprite.frameWidth = frameWidth ? frameWidth : width;
    sprite.frameHeight = frameHeight ? frameHeight : height;
    sprite.pixels = pixels;
    sprite.loaded = true;
    sprite.lastAccess = millis();
    
    m_cachedCount++;
    m_cacheUsage += pixelDataSize;
    
    LOG_VERBOSE("SpriteRenderer: Loaded '%s' (%dx%d, %d frames, %d bytes)",
                spriteId, width, height, frames, pixelDataSize);
    return true;
}

bool SpriteRenderer::loadSpriteSheet(const char* spriteId, const char* filePath,
                                      uint16_t frameWidth, uint16_t frameHeight, uint16_t frames) {
    // Load as regular sprite but with frame info
    if (loadSprite(spriteId, filePath)) {
        SpriteData* sprite = getSprite(spriteId);
        if (sprite) {
            sprite->frameWidth = frameWidth;
            sprite->frameHeight = frameHeight;
            sprite->frames = frames;
            return true;
        }
    }
    return false;
}

void SpriteRenderer::unloadSprite(const char* spriteId) {
    int8_t idx = findSpriteIndex(spriteId);
    if (idx < 0) return;
    
    if (m_cache[idx].pixels) {
        m_cacheUsage -= m_cache[idx].width * m_cache[idx].height * sizeof(uint16_t);
        free(m_cache[idx].pixels);
    }
    
    // Shift cache
    for (int i = idx; i < m_cachedCount - 1; i++) {
        m_cache[i] = m_cache[i + 1];
    }
    m_cachedCount--;
    
    LOG_VERBOSE("SpriteRenderer: Unloaded '%s'", spriteId);
}

void SpriteRenderer::unloadAll() {
    reset();
    LOG_VERBOSE("SpriteRenderer: All sprites unloaded");
}

bool SpriteRenderer::isSpriteLoaded(const char* spriteId) const {
    return findSpriteIndex(spriteId) >= 0;
}

SpriteData* SpriteRenderer::getSprite(const char* spriteId) {
    int8_t idx = findSpriteIndex(spriteId);
    if (idx < 0) return nullptr;
    
    m_cache[idx].lastAccess = millis();
    return &m_cache[idx];
}

// ===== Rendering =====

void SpriteRenderer::drawSprite(const SpriteRenderRequest& request) {
    if (m_batching) {
        if (m_batchCount < 64) {
            m_batchBuffer[m_batchCount++] = request;
        }
        return;
    }
    
    SpriteData* sprite = getSprite(request.spriteId);
    if (!sprite) return;
    
    renderSprite(sprite, request.x, request.y, request.frame,
                 request.flags, request.tintColor, request.alpha, request.scale);
}

void SpriteRenderer::drawSpriteFrame(const char* spriteId, int16_t x, int16_t y, uint8_t frame) {
    SpriteRenderRequest req;
    req.spriteId = spriteId;
    req.x = x;
    req.y = y;
    req.frame = frame;
    req.flags = 0;
    req.tintColor = 0;
    req.alpha = 255;
    req.scale = 1;
    drawSprite(req);
}

void SpriteRenderer::drawSpriteScaled(const char* spriteId, int16_t x, int16_t y, uint8_t frame, uint8_t scale) {
    SpriteRenderRequest req;
    req.spriteId = spriteId;
    req.x = x;
    req.y = y;
    req.frame = frame;
    req.flags = 0;
    req.tintColor = 0;
    req.alpha = 255;
    req.scale = scale;
    drawSprite(req);
}

void SpriteRenderer::drawSpriteTinted(const char* spriteId, int16_t x, int16_t y, uint8_t frame, uint16_t tintColor) {
    SpriteRenderRequest req;
    req.spriteId = spriteId;
    req.x = x;
    req.y = y;
    req.frame = frame;
    req.flags = SPRITE_TINTED;
    req.tintColor = tintColor;
    req.alpha = 255;
    req.scale = 1;
    drawSprite(req);
}

void SpriteRenderer::drawSpriteAlpha(const char* spriteId, int16_t x, int16_t y, uint8_t frame, uint8_t alpha) {
    SpriteRenderRequest req;
    req.spriteId = spriteId;
    req.x = x;
    req.y = y;
    req.frame = frame;
    req.flags = SPRITE_ALPHA;
    req.tintColor = 0;
    req.alpha = alpha;
    req.scale = 1;
    drawSprite(req);
}

void SpriteRenderer::drawSpriteFlipped(const char* spriteId, int16_t x, int16_t y, uint8_t frame, bool flipH, bool flipV) {
    SpriteRenderRequest req;
    req.spriteId = spriteId;
    req.x = x;
    req.y = y;
    req.frame = frame;
    req.flags = (flipH ? SPRITE_FLIP_H : 0) | (flipV ? SPRITE_FLIP_V : 0);
    req.tintColor = 0;
    req.alpha = 255;
    req.scale = 1;
    drawSprite(req);
}

// ===== Batch Rendering =====

void SpriteRenderer::beginBatch() {
    m_batching = true;
    m_batchCount = 0;
}

void SpriteRenderer::submitRequest(const SpriteRenderRequest& request) {
    if (m_batching && m_batchCount < 64) {
        m_batchBuffer[m_batchCount++] = request;
    }
}

void SpriteRenderer::endBatch() {
    m_batching = false;
    
    for (int i = 0; i < m_batchCount; i++) {
        const SpriteRenderRequest& req = m_batchBuffer[i];
        SpriteData* sprite = getSprite(req.spriteId);
        if (sprite) {
            renderSprite(sprite, req.x, req.y, req.frame,
                         req.flags, req.tintColor, req.alpha, req.scale);
        }
    }
    
    m_batchCount = 0;
}

// ===== Internal Rendering =====

void SpriteRenderer::renderSprite(const SpriteData* sprite, int16_t x, int16_t y,
                                   uint8_t frame, uint8_t flags, uint16_t tintColor,
                                   uint8_t alpha, uint8_t scale) {
    DisplayManager& display = DisplayManager::getInstance();
    
    // Calculate frame position in sprite sheet
    uint16_t frameCol = frame % (sprite->width / sprite->frameWidth);
    uint16_t frameRow = frame / (sprite->width / sprite->frameWidth);
    uint16_t frameX = frameCol * sprite->frameWidth;
    uint16_t frameY = frameRow * sprite->frameHeight;
    
    uint16_t fw = sprite->frameWidth;
    uint16_t fh = sprite->frameHeight;
    
    // Apply viewport clipping
    int16_t startX = max(x, (int16_t)0);
    int16_t startY = max(y, (int16_t)0);
    int16_t endX = min<int16_t>(x + fw * scale, (int16_t)DISPLAY_WIDTH);
    int16_t endY = min<int16_t>(y + fh * scale, (int16_t)DISPLAY_HEIGHT);
    
    bool flipH = (flags & SPRITE_FLIP_H) != 0;
    bool flipV = (flags & SPRITE_FLIP_V) != 0;
    bool useAlpha = (flags & SPRITE_ALPHA) && alpha < 255;
    bool useTint = (flags & SPRITE_TINTED) && tintColor != 0;
    
    for (int16_t sy = startY; sy < endY; sy++) {
        for (int16_t sx = startX; sx < endX; sx++) {
            // Calculate source pixel
            int16_t srcRelX = (sx - x) / scale;
            int16_t srcRelY = (sy - y) / scale;
            
            if (flipH) srcRelX = fw - 1 - srcRelX;
            if (flipV) srcRelY = fh - 1 - srcRelY;
            
            uint16_t srcX = frameX + srcRelX;
            uint16_t srcY = frameY + srcRelY;
            
            if (srcX >= sprite->width || srcY >= sprite->height) continue;
            
            uint16_t pixel = sprite->pixels[srcY * sprite->width + srcX];
            
            // Skip transparent pixels (color key = 0xF81F, hot pink)
            if (pixel == 0xF81F) continue;
            
            // Apply tint
            if (useTint) {
                // Simple tint: multiply colors
                uint8_t r = ((pixel >> 11) & 0x1F) * ((tintColor >> 11) & 0x1F) / 31;
                uint8_t g = ((pixel >> 5) & 0x3F) * ((tintColor >> 5) & 0x3F) / 63;
                uint8_t b = (pixel & 0x1F) * (tintColor & 0x1F) / 31;
                pixel = (r << 11) | (g << 5) | b;
            }
            
            // Apply alpha blending
            if (useAlpha) {
                uint16_t bg = display.readPixel(sx, sy);
                uint8_t r = (((pixel >> 11) & 0x1F) * alpha + ((bg >> 11) & 0x1F) * (255 - alpha)) / 255;
                uint8_t g = (((pixel >> 5) & 0x3F) * alpha + ((bg >> 5) & 0x3F) * (255 - alpha)) / 255;
                uint8_t b = ((pixel & 0x1F) * alpha + (bg & 0x1F) * (255 - alpha)) / 255;
                pixel = (r << 11) | (g << 5) | b;
            }
            
            display.drawPixel(sx, sy, pixel);
        }
    }
}

// ===== Cache Management =====

void SpriteRenderer::evictOldest() {
    if (m_cachedCount == 0) return;
    
    // Find least recently used sprite
    uint8_t oldestIdx = 0;
    uint32_t oldestTime = m_cache[0].lastAccess;
    
    for (int i = 1; i < m_cachedCount; i++) {
        if (m_cache[i].lastAccess < oldestTime) {
            oldestTime = m_cache[i].lastAccess;
            oldestIdx = i;
        }
    }
    
    LOG_VERBOSE("SpriteRenderer: Evicting '%s'", m_cache[oldestIdx].id);
    unloadSprite(m_cache[oldestIdx].id);
}

uint16_t SpriteRenderer::getCacheUsage() const {
    return m_cacheUsage;
}

// ===== Utility =====

uint16_t SpriteRenderer::getSpriteWidth(const char* spriteId) const {
    int8_t idx = findSpriteIndex(spriteId);
    if (idx < 0) return 0;
    return m_cache[idx].frameWidth;
}

uint16_t SpriteRenderer::getSpriteHeight(const char* spriteId) const {
    int8_t idx = findSpriteIndex(spriteId);
    if (idx < 0) return 0;
    return m_cache[idx].frameHeight;
}

uint8_t SpriteRenderer::getSpriteFrameCount(const char* spriteId) const {
    int8_t idx = findSpriteIndex(spriteId);
    if (idx < 0) return 0;
    return m_cache[idx].frames;
}

int8_t SpriteRenderer::findSpriteIndex(const char* spriteId) const {
    for (int i = 0; i < m_cachedCount; i++) {
        if (strcmp(m_cache[i].id, spriteId) == 0) {
            return i;
        }
    }
    return -1;
}