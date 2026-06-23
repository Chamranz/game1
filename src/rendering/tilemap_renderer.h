/*
 * NEON REQUIEM - Tilemap Renderer
 * Handles tilemap loading from SD, rendering with camera scrolling
 * Supports multiple layers, collision data, and animated tiles
 */

#ifndef TILEMAP_RENDERER_H
#define TILEMAP_RENDERER_H

#include <Arduino.h>
#include "config/config.h"

// Tilemap constants
#define MAX_TILEMAP_LAYERS 4
#define MAX_TILESET_CACHE 8
#define TILESET_CACHE_SIZE (256 * 1024)  // 256KB for tilesets
#define MAX_MAP_WIDTH 20
#define MAX_MAP_HEIGHT 15
#define TILE_SIZE 16  // 16x16 pixel tiles

// Tile flags
#define TILE_SOLID      0x01  // Collision
#define TILE_WATER      0x02  // Water (slow movement)
#define TILE_HAZARD     0x04  // Damage on contact
#define TILE_ICE        0x08  // Slippery
#define TILE_GRASS      0x10  // Stealth bonus
#define TILE_CONCRETE   0x20  // Footstep sound type
#define TILE_METAL      0x40  // Footstep sound type
#define TILE_INTERACT   0x80  // Can interact with

// Tile data structure
struct TileData {
    uint8_t tileIndex;     // Index into tileset
    uint8_t flags;         // Tile flags
    uint8_t animationFrame; // For animated tiles
    uint8_t animationSpeed; // Frames between animation updates
};

// Tilemap layer
struct TilemapLayer {
    TileData tiles[MAX_MAP_WIDTH * MAX_MAP_HEIGHT];
    uint16_t width;
    uint16_t height;
    bool visible;
    uint8_t parallaxFactor;  // 0-255, 128=normal, 0=static, 255=far background
    int16_t scrollOffsetX;
    int16_t scrollOffsetY;
};

// Tileset data
struct TilesetData {
    char id[20];
    uint16_t tileCount;
    uint16_t tileWidth;
    uint16_t tileHeight;
    uint16_t* pixels;  // PSRAM pointer to tile pixels
    bool loaded;
};

// Tilemap data
struct TilemapData {
    char id[20];
    char name[32];
    TilemapLayer layers[MAX_TILEMAP_LAYERS];
    uint8_t layerCount;
    uint16_t width;
    uint16_t height;
    uint16_t backgroundColor;
    
    // Spawn points
    int16_t playerSpawnX;
    int16_t playerSpawnY;
    
    // Connected maps
    char northMap[20];
    char southMap[20];
    char eastMap[20];
    char westMap[20];
};

class TilemapRenderer {
public:
    // Singleton access
    static TilemapRenderer& getInstance();
    
    // Initialization
    void begin();
    void reset();
    
    // Tileset management
    bool loadTileset(const char* tilesetId, const char* filePath, 
                     uint16_t tileWidth, uint16_t tileHeight, uint16_t tileCount);
    void unloadTileset(const char* tilesetId);
    bool isTilesetLoaded(const char* tilesetId) const;
    
    // Tilemap loading
    bool loadTilemap(const char* mapId, const char* filePath);
    void unloadTilemap();
    TilemapData* getCurrentTilemap() { return m_currentMap ? &m_currentMapData : nullptr; }
    bool isMapLoaded() const { return m_currentMap; }
    
    // Rendering
    void render(int16_t cameraX, int16_t cameraY);
    void renderLayer(uint8_t layerIndex, int16_t cameraX, int16_t cameraY);
    void renderTile(uint8_t layerIndex, uint16_t tileX, uint16_t tileY, int16_t screenX, int16_t screenY);
    
    // Collision
    bool isTileSolid(uint16_t tileX, uint16_t tileY, uint8_t layerIndex = 0) const;
    bool isTileHazard(uint16_t tileX, uint16_t tileY, uint8_t layerIndex = 0) const;
    bool isTileWater(uint16_t tileX, uint16_t tileY, uint8_t layerIndex = 0) const;
    uint8_t getTileFlags(uint16_t tileX, uint16_t tileY, uint8_t layerIndex = 0) const;
    TileData getTile(uint16_t tileX, uint16_t tileY, uint8_t layerIndex = 0) const;
    
    // Map dimensions
    uint16_t getMapWidth() const { return m_currentMapData.width; }
    uint16_t getMapHeight() const { return m_currentMapData.height; }
    uint16_t getMapPixelWidth() const { return m_currentMapData.width * TILE_SIZE; }
    uint16_t getMapPixelHeight() const { return m_currentMapData.height * TILE_SIZE; }
    
    // Spawn points
    int16_t getSpawnX() const { return m_currentMapData.playerSpawnX; }
    int16_t getSpawnY() const { return m_currentMapData.playerSpawnY; }
    
    // Map connections
    const char* getNorthMap() const { return m_currentMapData.northMap; }
    const char* getSouthMap() const { return m_currentMapData.southMap; }
    const char* getEastMap() const { return m_currentMapData.eastMap; }
    const char* getWestMap() const { return m_currentMapData.westMap; }
    
    // Animated tiles
    void updateAnimations();
    
    // Utility
    void worldToTile(int16_t worldX, int16_t worldY, uint16_t& tileX, uint16_t& tileY) const;
    void tileToWorld(uint16_t tileX, uint16_t tileY, int16_t& worldX, int16_t& worldY) const;

private:
    TilemapRenderer() : m_currentMap(false), m_tilesetCount(0), m_animTimer(0) {}
    ~TilemapRenderer() {}
    TilemapRenderer(const TilemapRenderer&) = delete;
    TilemapRenderer& operator=(const TilemapRenderer&) = delete;
    
    // Find tileset
    int8_t findTilesetIndex(const char* tilesetId) const;
    
    TilesetData m_tilesets[MAX_TILESET_CACHE];
    uint8_t m_tilesetCount;
    
    TilemapData m_currentMapData;
    bool m_currentMap;
    
    uint16_t m_animTimer;
};

#endif // TILEMAP_RENDERER_H