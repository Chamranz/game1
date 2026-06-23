/*
 * NEON REQUIEM - Save System
 * Handles save/load game state to SD card with CRC validation
 * Supports 3 save slots with metadata
 */

#ifndef SAVE_SYSTEM_H
#define SAVE_SYSTEM_H

#include <Arduino.h>
#include "config/config.h"

// Save system constants
#define MAX_SAVE_SLOTS 3
#define SAVE_MAGIC 0x4E5251  // "NRQ" magic number
#define SAVE_VERSION 0x0100  // Version 1.0
#define SAVE_HEADER_SIZE 32  // Bytes reserved for header
#define SAVE_DATA_SIZE 4096  // Max save data size per slot
#define SAVE_FILENAME_LEN 24

// Save slot metadata
struct SaveSlotMeta {
    bool used;                  // Slot has save data
    uint8_t slotNumber;         // 0-2
    char playerName[16];        // Player name (KAI)
    uint8_t level;              // Player level
    uint16_t playTime;          // Play time in minutes
    uint8_t currentArc;         // Current story arc
    GameRoute currentRoute;     // Current game route
    char location[24];          // Last location name
    uint32_t timestamp;         // Unix timestamp of save
    uint16_t checksum;          // CRC-16 of save data
};

// Save header structure (written to SD)
struct SaveHeader {
    uint32_t magic;             // Magic number for validation
    uint16_t version;           // Save format version
    uint16_t dataSize;          // Size of save data
    uint16_t checksum;          // CRC-16 of entire save
    uint8_t reserved[SAVE_HEADER_SIZE - 10]; // Padding
};

// Save slot information for UI
struct SaveSlotInfo {
    bool exists;
    SaveSlotMeta meta;
};

class SaveSystem {
public:
    // Singleton access
    static SaveSystem& getInstance();
    
    // Initialization
    void begin();
    void reset();
    
    // Save slot management
    bool saveToSlot(uint8_t slot, const uint8_t* data, uint16_t dataSize);
    bool loadFromSlot(uint8_t slot, uint8_t* data, uint16_t maxSize);
    bool deleteSlot(uint8_t slot);
    bool slotExists(uint8_t slot) const;
    
    // Save slot metadata
    SaveSlotMeta getSlotMeta(uint8_t slot) const;
    void updateSlotMeta(uint8_t slot, const SaveSlotMeta& meta);
    SaveSlotInfo getSlotInfo(uint8_t slot);
    
    // Quick save/load (uses current slot)
    bool quickSave(const uint8_t* data, uint16_t dataSize);
    bool quickLoad(uint8_t* data, uint16_t maxSize);
    
    // Current slot management
    uint8_t getCurrentSlot() const { return m_currentSlot; }
    void setCurrentSlot(uint8_t slot) { 
        if (slot < MAX_SAVE_SLOTS) m_currentSlot = slot; 
    }
    
    // Auto-save
    bool autoSave(const uint8_t* data, uint16_t dataSize);
    bool hasAutoSave() const;
    bool loadAutoSave(uint8_t* data, uint16_t maxSize);
    
    // Save data size management
    uint16_t getMaxSaveSize() const { return SAVE_DATA_SIZE; }
    void setSaveDataSize(uint16_t size) { m_saveDataSize = min(size, (uint16_t)SAVE_DATA_SIZE); }
    
    // Utility
    const char* getSlotFilename(uint8_t slot) const;
    const char* getAutoSaveFilename() const { return "/saves/autosave.sav"; }
    void formatTimestamp(char* buffer, size_t bufferSize, uint32_t timestamp) const;
    
    // Error handling
    bool getLastError() const { return m_lastError; }
    const char* getLastErrorString() const;

private:
    SaveSystem() : m_currentSlot(0), m_saveDataSize(SAVE_DATA_SIZE), 
                   m_lastError(false), m_initialized(false) {}
    ~SaveSystem() {}
    SaveSystem(const SaveSystem&) = delete;
    SaveSystem& operator=(const SaveSystem&) = delete;
    
    // CRC-16 calculation
    uint16_t calculateCRC16(const uint8_t* data, uint16_t length) const;
    
    // Internal save/load
    bool writeSaveFile(const char* filename, const uint8_t* data, uint16_t dataSize, const SaveSlotMeta& meta);
    bool readSaveFile(const char* filename, uint8_t* data, uint16_t maxSize, SaveSlotMeta& meta);
    
    // Slot metadata cache
    SaveSlotMeta m_slotMeta[MAX_SAVE_SLOTS];
    bool m_slotMetaValid[MAX_SAVE_SLOTS];
    
    uint8_t m_currentSlot;
    uint16_t m_saveDataSize;
    bool m_lastError;
    bool m_initialized;
    
    // Filename cache
    char m_filenames[MAX_SAVE_SLOTS][SAVE_FILENAME_LEN];
};

#endif // SAVE_SYSTEM_H