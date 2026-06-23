/*
 * NEON REQUIEM - Tilemap Renderer Implementation
 * Handles tilemap loading from SD, rendering with camera scrolling
 */

#include "tilemap_renderer.h"
#include "../core/display/display_manager.h"
#include "../core/storage/sd_manager.h"

// Singleton instance
TilemapRenderer& TilemapRenderer::getInstance() {
    static TilemapRenderer instance;
    return instance;
}

void TilemapRenderer::begin() {
    reset();
    LOG_INFO("TilemapRenderer: Initialized (%d tileset cache, %dKB)", 
             MAX_TILESET_CACHE, TILESET_CACHE_SIZE / 1024);
}

void TilemapRenderer::reset() {
    // Free tileset PSRAM
    for (int i = 0; i < m_tilesetCount; i++) {
        if (m_tilesets[i].pixels) {
            free(m_tilesets[i].pixels);
            m_tilesets[i].pixels = nullptr;
        }
    }
    m_tilesetCount = 0;
    m_currentMap = false;
    m_animTimer = 0;
}

// ===== Tileset Management =====

bool TilemapRenderer::loadTileset(const char* tilesetId, const char* filePath,
                                   uint16_t tileWidth, uint16_t tileHeight, uint16_t tileCount) {
    SDManager& sd = SDManager::getInstance();
    
    if (m_tilesetCount >= MAX_TILESET_CACHE) {
        LOG_WARN("TilemapRenderer: Tileset cache full");
        return false;
    }
    
    // Read tileset file
    size_t fileSize = sd.getFileSize(filePath);
    if (fileSize == 0) {
        LOG_ERROR("TilemapRenderer: Tileset '%s' not found", filePath);
        return false;
    }
    
    // Calculate pixel data size
    uint16_t tilePixelCount = tileWidth * tileHeight;
    uint16_t totalPixels = tilePixelCount * tileCount;
    uint16_t pixelDataSize = totalPixels * sizeof(uint16_t);
    
    // Allocate PSRAM for pixel data
    uint16_t* pixels = (uint16_t*)ps_malloc(pixelDataSize);
    if (!pixels) {
        LOG_ERROR("TilemapRenderer: Failed to allocate tileset '%s' (%d bytes)",
                  tilesetId, pixelDataSize);
        return false;
    }
    
    // Read file data using SD manager
    uint8_t* fileBuffer = sd.readFile(filePath, fileSize);
    if (!fileBuffer) {
        LOG_ERROR("TilemapRenderer: Failed to read file '%s'", filePath);
        free(pixels);
        return false;
    }
    
    // Parse: header(8 bytes: tileW(2)+tileH(2)+count(2)+reserved(2)) + pixels
    memcpy(pixels, fileBuffer + 8, pixelDataSize);
    free(fileBuffer);
    
    // Store tileset
    TilesetData& ts = m_tilesets[m_tilesetCount];
    strncpy(ts.id, tilesetId, sizeof(ts.id) - 1);
    ts.id[sizeof(ts.id) - 1] = '\0';
    ts.tileCount = tileCount;
    ts.tileWidth = tileWidth;
    ts.tileHeight = tileHeight;
    ts.pixels = pixels;
    ts.loaded = true;
    m_tilesetCount++;
    
    LOG_VERBOSE("TilemapRenderer: Loaded tileset '%s' (%d tiles, %d bytes)",
                tilesetId, tileCount, pixelDataSize);
    return true;
}

void TilemapRenderer::unloadTileset(const char* tilesetId) {
    int8_t idx = findTilesetIndex(tilesetId);
    if (idx < 0) return;
    
    if (m_tilesets[idx].pixels) {
        free(m_tilesets[idx].pixels);
    }
    
    for (int i = idx; i < m_tilesetCount - 1; i++) {
        m_tilesets[i] = m_tilesets[i + 1];
    }
    m_tilesetCount--;
}

bool TilemapRenderer::isTilesetLoaded(const char* tilesetId) const {
    return findTilesetIndex(tilesetId) >= 0;
}

// ===== Tilemap Loading =====

bool TilemapRenderer::loadTilemap(const char* mapId, const char* filePath) {
    SDManager& sd = SDManager::getInstance();
    
    size_t fileSize = sd.getFileSize(filePath);
    if (fileSize == 0) {
        LOG_ERROR("TilemapRenderer: Map '%s' not found", filePath);
        return false;
    }
    
    // Read map file using SD manager
    uint8_t* buffer = sd.readFile(filePath, fileSize);
    if (!buffer) {
        LOG_ERROR("TilemapRenderer: Failed to read map file '%s'", filePath);
        return false;
    }
    
    // Parse map data (binary format)
    uint16_t offset = 0;
    
    // Map header
    uint16_t mapWidth = buffer[offset] | (buffer[offset + 1] << 8); offset += 2;
    uint16_t mapHeight = buffer[offset] | (buffer[offset + 1] << 8); offset += 2;
    uint8_t layerCount = buffer[offset++];
    uint16_t bgColor = buffer[offset] | (buffer[offset + 1] << 8); offset += 2;
    
    if (mapWidth > MAX_MAP_WIDTH || mapHeight > MAX_MAP_HEIGHT || layerCount > MAX_TILEMAP_LAYERS) {
        free(buffer);
        LOG_ERROR("TilemapRenderer: Invalid map dimensions (%dx%d, %d layers)",
                  mapWidth, mapHeight, layerCount);
        return false;
    }
    
    // Clear current map
    m_currentMapData = TilemapData();
    
    // Set map header
    strncpy(m_currentMapData.id, mapId, sizeof(m_currentMapData.id) - 1);
    m_currentMapData.width = mapWidth;
    m_currentMapData.height = mapHeight;
    m_currentMapData.layerCount = layerCount;
    m_currentMapData.backgroundColor = bgColor;
    
    // Read spawn points
    m_currentMapData.playerSpawnX = buffer[offset] | (buffer[offset + 1] << 8); offset += 2;
    m_currentMapData.playerSpawnY = buffer[offset] | (buffer[offset + 1] << 8); offset += 2;
    
    // Read connections
    memcpy(m_currentMapData.northMap, buffer + offset, 20); offset += 20;
    memcpy(m_currentMapData.southMap, buffer + offset, 20); offset += 20;
    memcpy(m_currentMapData.eastMap, buffer + offset, 20); offset += 20;
    memcpy(m_currentMapData.westMap, buffer + offset, 20); offset += 20;
    
    // Read layers
    for (int l = 0; l < layerCount; l++) {
        TilemapLayer& layer = m_currentMapData.layers[l];
        layer.width = buffer[offset] | (buffer[offset + 1] << 8); offset += 2;
        layer.height = buffer[offset] | (buffer[offset + 1] << 8); offset += 2;
        layer.visible = buffer[offset++] != 0;
        layer.parallaxFactor = buffer[offset++];
        layer.scrollOffsetX = 0;
        layer.scrollOffsetY = 0;
        
        // Read tile data
        uint16_t tileCount = layer.width * layer.height;
        for (int t = 0; t < tileCount; t++) {
            layer.tiles[t].tileIndex = buffer[offset++];
            layer.tiles[t].flags = buffer[offset++];
            layer.tiles[t].animationFrame = 0;
            layer.tiles[t].animationSpeed = buffer[offset++];
        }
    }
    
    free(buffer);
    m_currentMap = true;
    
    LOG_INFO("TilemapRenderer: Loaded map '%s' (%dx%d, %d layers)",
             mapId, mapWidth, mapHeight, layerCount);
    return true;
}

void TilemapRenderer::unloadTilemap() {
    m_currentMap = false;
}

// ===== Rendering =====

void TilemapRenderer::render(int16_t cameraX, int16_t cameraY) {
    if (!m_currentMap) return;
    
    DisplayManager& display = DisplayManager::getInstance();
    
    // Clear screen with background color
    display.fillScreen(m_currentMapData.backgroundColor);
    
    // Render each layer
    for (int l = 0; l < m_currentMapData.layerCount; l++) {
        if (m_currentMapData.layers[l].visible) {
            renderLayer(l, cameraX, cameraY);
        }
    }
}

void TilemapRenderer::renderLayer(uint8_t layerIndex, int16_t cameraX, int16_t cameraY) {
    if (!m_currentMap || layerIndex >= m_currentMapData.layerCount) return;
    
    TilemapLayer& layer = m_currentMapData.layers[layerIndex];
    
    // Apply parallax
    int16_t offsetX = cameraX * layer.parallaxFactor / 128 + layer.scrollOffsetX;
    int16_t offsetY = cameraY * layer.parallaxFactor / 128 + layer.scrollOffsetY;
    
    // Calculate visible tile range
    int16_t startTileX = max(offsetX / TILE_SIZE, 0);
    int16_t startTileY = max(offsetY / TILE_SIZE, 0);
    int16_t endTileX = min<int16_t>((offsetX + DISPLAY_WIDTH) / TILE_SIZE + 1, (int16_t)layer.width);
    int16_t endTileY = min<int16_t>((offsetY + DISPLAY_HEIGHT) / TILE_SIZE + 1, (int16_t)layer.height);
    
    // Render visible tiles
    for (int ty = startTileY; ty < endTileY; ty++) {
        for (int tx = startTileX; tx < endTileX; tx++) {
            int16_t screenX = tx * TILE_SIZE - offsetX;
            int16_t screenY = ty * TILE_SIZE - offsetY;
            renderTile(layerIndex, tx, ty, screenX, screenY);
        }
    }
}

void TilemapRenderer::renderTile(uint8_t layerIndex, uint16_t tileX, uint16_t tileY, 
                                  int16_t screenX, int16_t screenY) {
    if (!m_currentMap || layerIndex >= m_currentMapData.layerCount) return;
    
    TileData& tile = m_currentMapData.layers[layerIndex].tiles[tileY * m_currentMapData.layers[layerIndex].width + tileX];
    
    if (tile.tileIndex == 0) return; // Empty tile
    
    // Find tileset (using first loaded tileset for now)
    if (m_tilesetCount == 0) return;
    
    TilesetData& ts = m_tilesets[0];
    if (!ts.pixels) return;
    
    // Calculate tile pixel position in tileset
    uint16_t tilePixelX = (tile.tileIndex % (ts.tileWidth > 0 ? (ts.tileCount * ts.tileWidth) / ts.tileHeight : 1)) * ts.tileWidth;
    uint16_t tilePixelY = (tile.tileIndex / (ts.tileWidth > 0 ? (ts.tileCount * ts.tileWidth) / ts.tileHeight : 1)) * ts.tileHeight;
    
    DisplayManager& display = DisplayManager::getInstance();
    
    // Draw tile pixels
    for (int y = 0; y < ts.tileHeight; y++) {
        for (int x = 0; x < ts.tileWidth; x++) {
            int16_t drawX = screenX + x;
            int16_t drawY = screenY + y;
            
            if (drawX < 0 || drawX >= DISPLAY_WIDTH || drawY < 0 || drawY >= DISPLAY_HEIGHT) continue;
            
            uint16_t pixel = ts.pixels[(tilePixelY + y) * (ts.tileCount * ts.tileWidth) + (tilePixelX + x)];
            
            // Skip transparent
            if (pixel == 0xF81F) continue;
            
            display.drawPixel(drawX, drawY, pixel);
        }
    }
}

// ===== Collision =====

bool TilemapRenderer::isTileSolid(uint16_t tileX, uint16_t tileY, uint8_t layerIndex) const {
    if (!m_currentMap || layerIndex >= m_currentMapData.layerCount) return false;
    const TilemapLayer& layer = m_currentMapData.layers[layerIndex];
    if (tileX >= layer.width || tileY >= layer.height) return true; // Out of bounds = solid
    return (layer.tiles[tileY * layer.width + tileX].flags & TILE_SOLID) != 0;
}

bool TilemapRenderer::isTileHazard(uint16_t tileX, uint16_t tileY, uint8_t layerIndex) const {
    if (!m_currentMap || layerIndex >= m_currentMapData.layerCount) return false;
    const TilemapLayer& layer = m_currentMapData.layers[layerIndex];
    if (tileX >= layer.width || tileY >= layer.height) return false;
    return (layer.tiles[tileY * layer.width + tileX].flags & TILE_HAZARD) != 0;
}

bool TilemapRenderer::isTileWater(uint16_t tileX, uint16_t tileY, uint8_t layerIndex) const {
    if (!m_currentMap || layerIndex >= m_currentMapData.layerCount) return false;
    const TilemapLayer& layer = m_currentMapData.layers[layerIndex];
    if (tileX >= layer.width || tileY >= layer.height) return false;
    return (layer.tiles[tileY * layer.width + tileX].flags & TILE_WATER) != 0;
}

uint8_t TilemapRenderer::getTileFlags(uint16_t tileX, uint16_t tileY, uint8_t layerIndex) const {
    if (!m_currentMap || layerIndex >= m_currentMapData.layerCount) return 0;
    const TilemapLayer& layer = m_currentMapData.layers[layerIndex];
    if (tileX >= layer.width || tileY >= layer.height) return TILE_SOLID;
    return layer.tiles[tileY * layer.width + tileX].flags;
}

TileData TilemapRenderer::getTile(uint16_t tileX, uint16_t tileY, uint8_t layerIndex) const {
    TileData empty = {0};
    if (!m_currentMap || layerIndex >= m_currentMapData.layerCount) return empty;
    const TilemapLayer& layer = m_currentMapData.layers[layerIndex];
    if (tileX >= layer.width || tileY >= layer.height) return empty;
    return layer.tiles[tileY * layer.width + tileX];
}

// ===== Animated Tiles =====

void TilemapRenderer::updateAnimations() {
    if (!m_currentMap) return;
    
    m_animTimer++;
    if (m_animTimer < 4) return; // Update every 4 frames
    m_animTimer = 0;
    
    for (int l = 0; l < m_currentMapData.layerCount; l++) {
        TilemapLayer& layer = m_currentMapData.layers[l];
        for (int t = 0; t < layer.width * layer.height; t++) {
            if (layer.tiles[t].animationSpeed > 0) {
                layer.tiles[t].animationFrame++;
                if (layer.tiles[t].animationFrame >= layer.tiles[t].animationSpeed) {
                    layer.tiles[t].animationFrame = 0;
                    // Cycle through animation frames (tileIndex + 1, +2, etc.)
                    // Base tile index is stored, animation cycles through next tiles
                }
            }
        }
    }
}

// ===== Utility =====

void TilemapRenderer::worldToTile(int16_t worldX, int16_t worldY, uint16_t& tileX, uint16_t& tileY) const {
    tileX = worldX / TILE_SIZE;
    tileY = worldY / TILE_SIZE;
}

void TilemapRenderer::tileToWorld(uint16_t tileX, uint16_t tileY, int16_t& worldX, int16_t& worldY) const {
    worldX = tileX * TILE_SIZE + TILE_SIZE / 2;
    worldY = tileY * TILE_SIZE + TILE_SIZE / 2;
}

int8_t TilemapRenderer::findTilesetIndex(const char* tilesetId) const {
    for (int i = 0; i < m_tilesetCount; i++) {
        if (strcmp(m_tilesets[i].id, tilesetId) == 0) {
            return i;
        }
    }
    return -1;
}