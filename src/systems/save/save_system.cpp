/*
 * NEON REQUIEM - Save System Implementation
 * Handles save/load game state to SD card with CRC validation
 */

#include "save_system.h"
#include "../../core/storage/sd_manager.h"
#include <SD.h>

// Singleton instance
SaveSystem& SaveSystem::getInstance() {
    static SaveSystem instance;
    return instance;
}

void SaveSystem::begin() {
    reset();
    
    // Initialize filenames
    for (int i = 0; i < MAX_SAVE_SLOTS; i++) {
        snprintf(m_filenames[i], SAVE_FILENAME_LEN, "/saves/slot%d.sav", i);
    }
    
    // Check if saves directory exists, create if not
    if (!SD.exists("/saves")) {
        SD.mkdir("/saves");
    }
    
    // Validate existing save slots
    for (int i = 0; i < MAX_SAVE_SLOTS; i++) {
        m_slotMetaValid[i] = false;
        if (SD.exists(m_filenames[i])) {
            // Read just the header to validate
            SaveHeader header;
            File file = SD.open(m_filenames[i], FILE_READ);
            if (file) {
                if (file.read((uint8_t*)&header, sizeof(SaveHeader)) == sizeof(SaveHeader) && header.magic == SAVE_MAGIC) {
                    m_slotMeta[i].used = true;
                    m_slotMeta[i].slotNumber = i;
                    // Read metadata after header
                    file.read((uint8_t*)&m_slotMeta[i], sizeof(SaveSlotMeta));
                    m_slotMetaValid[i] = true;
                }
                file.close();
            }
        } else {
            m_slotMeta[i].used = false;
            m_slotMeta[i].slotNumber = i;
        }
    }
    
    m_initialized = true;
    m_lastError = false;
    
    LOG_INFO("SaveSystem: Initialized with %d slots", MAX_SAVE_SLOTS);
}

void SaveSystem::reset() {
    m_currentSlot = 0;
    m_saveDataSize = SAVE_DATA_SIZE;
    m_lastError = false;
    m_initialized = false;
    
    for (int i = 0; i < MAX_SAVE_SLOTS; i++) {
        m_slotMeta[i].used = false;
        m_slotMeta[i].slotNumber = i;
        m_slotMeta[i].level = 0;
        m_slotMeta[i].playTime = 0;
        m_slotMeta[i].currentArc = 0;
        m_slotMeta[i].currentRoute = GameRoute::RESONANCE;
        m_slotMeta[i].timestamp = 0;
        m_slotMeta[i].checksum = 0;
        m_slotMetaValid[i] = false;
        strcpy(m_slotMeta[i].playerName, "KAI");
        strcpy(m_slotMeta[i].location, "Unknown");
    }
}

// ===== CRC-16 Calculation =====

uint16_t SaveSystem::calculateCRC16(const uint8_t* data, uint16_t length) const {
    uint16_t crc = 0xFFFF;
    for (uint16_t i = 0; i < length; i++) {
        crc ^= data[i];
        for (int j = 0; j < 8; j++) {
            if (crc & 0x0001) {
                crc = (crc >> 1) ^ 0xA001;
            } else {
                crc >>= 1;
            }
        }
    }
    return crc;
}

// ===== Internal Save/Load =====

bool SaveSystem::writeSaveFile(const char* filename, const uint8_t* data, uint16_t dataSize, const SaveSlotMeta& meta) {
    // Calculate total file size
    uint16_t totalSize = sizeof(SaveHeader) + sizeof(SaveSlotMeta) + dataSize;
    
    // Allocate buffer for save file
    uint8_t* buffer = (uint8_t*)ps_malloc(totalSize);
    if (!buffer) {
        LOG_ERROR("SaveSystem: Failed to allocate save buffer (%d bytes)", totalSize);
        m_lastError = true;
        return false;
    }
    
    // Build header
    SaveHeader header;
    header.magic = SAVE_MAGIC;
    header.version = SAVE_VERSION;
    header.dataSize = dataSize;
    header.checksum = 0; // Will calculate after filling
    
    // Copy header
    memcpy(buffer, &header, sizeof(SaveHeader));
    
    // Copy metadata
    memcpy(buffer + sizeof(SaveHeader), &meta, sizeof(SaveSlotMeta));
    
    // Copy save data
    memcpy(buffer + sizeof(SaveHeader) + sizeof(SaveSlotMeta), data, dataSize);
    
    // Calculate CRC over everything except the checksum field itself
    uint16_t crc = calculateCRC16(buffer, totalSize);
    header.checksum = crc;
    
    // Update header with checksum
    memcpy(buffer, &header, sizeof(SaveHeader));
    
    // Write to SD using raw SD library
    File file = SD.open(filename, FILE_WRITE);
    bool success = false;
    if (file) {
        success = (file.write(buffer, totalSize) == totalSize);
        file.close();
    } else {
        LOG_ERROR("SaveSystem: Failed to open '%s' for writing", filename);
    }
    
    free(buffer);
    
    if (!success) {
        LOG_ERROR("SaveSystem: Failed to write save file '%s'", filename);
        m_lastError = true;
        return false;
    }
    
    m_lastError = false;
    return true;
}

bool SaveSystem::readSaveFile(const char* filename, uint8_t* data, uint16_t maxSize, SaveSlotMeta& meta) {
    if (!SD.exists(filename)) {
        LOG_WARN("SaveSystem: Save file '%s' not found", filename);
        m_lastError = true;
        return false;
    }
    
    File file = SD.open(filename, FILE_READ);
    if (!file) {
        LOG_ERROR("SaveSystem: Failed to open '%s' for reading", filename);
        m_lastError = true;
        return false;
    }
    
    size_t fileSize = file.size();
    if (fileSize < sizeof(SaveHeader) + sizeof(SaveSlotMeta)) {
        LOG_ERROR("SaveSystem: Corrupt save file '%s' (too small)", filename);
        file.close();
        m_lastError = true;
        return false;
    }
    
    // Read header
    SaveHeader header;
    if (file.read((uint8_t*)&header, sizeof(SaveHeader)) != sizeof(SaveHeader)) {
        LOG_ERROR("SaveSystem: Failed to read save header");
        file.close();
        m_lastError = true;
        return false;
    }
    
    // Validate magic number
    if (header.magic != SAVE_MAGIC) {
        LOG_ERROR("SaveSystem: Invalid save file magic (0x%04X)", (unsigned int)header.magic);
        file.close();
        m_lastError = true;
        return false;
    }
    
    // Validate version
    if (header.version != SAVE_VERSION) {
        LOG_WARN("SaveSystem: Save version mismatch (file: 0x%04X, expected: 0x%04X)", 
                 header.version, SAVE_VERSION);
        // Try to load anyway
    }
    
    // Read metadata
    if (file.read((uint8_t*)&meta, sizeof(SaveSlotMeta)) != sizeof(SaveSlotMeta)) {
        LOG_ERROR("SaveSystem: Failed to read save metadata");
        file.close();
        m_lastError = true;
        return false;
    }
    
    // Read save data
    uint16_t dataToRead = (header.dataSize < maxSize) ? header.dataSize : maxSize;
    if (file.read(data, dataToRead) != dataToRead) {
        LOG_ERROR("SaveSystem: Failed to read save data");
        file.close();
        m_lastError = true;
        return false;
    }
    
    file.close();
    
    // Verify CRC (read entire file to verify)
    file = SD.open(filename, FILE_READ);
    if (file) {
        uint8_t* verifyBuf = (uint8_t*)ps_malloc(fileSize);
        if (verifyBuf) {
            file.read(verifyBuf, fileSize);
            
            // Zero out checksum in header for verification
            SaveHeader* hdr = (SaveHeader*)verifyBuf;
            hdr->checksum = 0;
            
            uint16_t expectedCRC = header.checksum;
            uint16_t actualCRC = calculateCRC16(verifyBuf, fileSize);
            
            if (expectedCRC != actualCRC) {
                LOG_WARN("SaveSystem: CRC mismatch (expected: 0x%04X, actual: 0x%04X)", 
                         expectedCRC, actualCRC);
                // Data might be corrupt, but we'll return it anyway
            }
            
            free(verifyBuf);
        }
        file.close();
    }
    
    m_lastError = false;
    return true;
}

// ===== Save Slot Management =====

bool SaveSystem::saveToSlot(uint8_t slot, const uint8_t* data, uint16_t dataSize) {
    if (slot >= MAX_SAVE_SLOTS) {
        LOG_ERROR("SaveSystem: Invalid slot %d", slot);
        m_lastError = true;
        return false;
    }
    
    if (dataSize > SAVE_DATA_SIZE) {
        LOG_ERROR("SaveSystem: Data too large for slot (%d > %d)", dataSize, SAVE_DATA_SIZE);
        m_lastError = true;
        return false;
    }
    
    // Update metadata
    m_slotMeta[slot].used = true;
    m_slotMeta[slot].slotNumber = slot;
    m_slotMeta[slot].timestamp = millis();
    m_slotMeta[slot].checksum = calculateCRC16(data, dataSize);
    m_slotMetaValid[slot] = true;
    
    bool success = writeSaveFile(m_filenames[slot], data, dataSize, m_slotMeta[slot]);
    
    if (success) {
        LOG_INFO("SaveSystem: Saved to slot %d (%d bytes)", slot, dataSize);
    }
    
    return success;
}

bool SaveSystem::loadFromSlot(uint8_t slot, uint8_t* data, uint16_t maxSize) {
    if (slot >= MAX_SAVE_SLOTS) {
        LOG_ERROR("SaveSystem: Invalid slot %d", slot);
        m_lastError = true;
        return false;
    }
    
    SaveSlotMeta meta;
    bool success = readSaveFile(m_filenames[slot], data, maxSize, meta);
    
    if (success) {
        m_slotMeta[slot] = meta;
        m_slotMetaValid[slot] = true;
        LOG_INFO("SaveSystem: Loaded from slot %d", slot);
    }
    
    return success;
}

bool SaveSystem::deleteSlot(uint8_t slot) {
    if (slot >= MAX_SAVE_SLOTS) {
        m_lastError = true;
        return false;
    }
    
    bool success = SD.remove(m_filenames[slot]);
    
    if (success) {
        m_slotMeta[slot].used = false;
        m_slotMetaValid[slot] = false;
        LOG_INFO("SaveSystem: Deleted slot %d", slot);
    }
    
    return success;
}

bool SaveSystem::slotExists(uint8_t slot) const {
    if (slot >= MAX_SAVE_SLOTS) return false;
    return m_slotMeta[slot].used;
}

// ===== Slot Metadata =====

SaveSlotMeta SaveSystem::getSlotMeta(uint8_t slot) const {
    if (slot >= MAX_SAVE_SLOTS) {
        SaveSlotMeta empty = {0};
        return empty;
    }
    return m_slotMeta[slot];
}

void SaveSystem::updateSlotMeta(uint8_t slot, const SaveSlotMeta& meta) {
    if (slot >= MAX_SAVE_SLOTS) return;
    m_slotMeta[slot] = meta;
    m_slotMetaValid[slot] = true;
}

SaveSlotInfo SaveSystem::getSlotInfo(uint8_t slot) {
    SaveSlotInfo info;
    info.exists = slotExists(slot);
    info.meta = getSlotMeta(slot);
    return info;
}

// ===== Quick Save/Load =====

bool SaveSystem::quickSave(const uint8_t* data, uint16_t dataSize) {
    return saveToSlot(m_currentSlot, data, dataSize);
}

bool SaveSystem::quickLoad(uint8_t* data, uint16_t maxSize) {
    return loadFromSlot(m_currentSlot, data, maxSize);
}

// ===== Auto-Save =====

bool SaveSystem::autoSave(const uint8_t* data, uint16_t dataSize) {
    if (dataSize > SAVE_DATA_SIZE) {
        m_lastError = true;
        return false;
    }
    
    SaveSlotMeta meta;
    memset(&meta, 0, sizeof(SaveSlotMeta));
    meta.used = true;
    meta.slotNumber = 0xFF; // Auto-save indicator
    meta.timestamp = millis();
    meta.checksum = calculateCRC16(data, dataSize);
    strcpy(meta.playerName, "KAI");
    strcpy(meta.location, "Auto-Save");
    
    bool success = writeSaveFile(getAutoSaveFilename(), data, dataSize, meta);
    
    if (success) {
        LOG_INFO("SaveSystem: Auto-saved (%d bytes)", dataSize);
    }
    
    return success;
}

bool SaveSystem::hasAutoSave() const {
    return SD.exists(getAutoSaveFilename());
}

bool SaveSystem::loadAutoSave(uint8_t* data, uint16_t maxSize) {
    SaveSlotMeta meta;
    bool success = readSaveFile(getAutoSaveFilename(), data, maxSize, meta);
    
    if (success) {
        LOG_INFO("SaveSystem: Auto-save loaded");
    }
    
    return success;
}

// ===== Utility =====

const char* SaveSystem::getSlotFilename(uint8_t slot) const {
    if (slot >= MAX_SAVE_SLOTS) return nullptr;
    return m_filenames[slot];
}

void SaveSystem::formatTimestamp(char* buffer, size_t bufferSize, uint32_t timestamp) const {
    // Simple timestamp formatting (minutes:seconds since save)
    uint16_t minutes = (timestamp / 60000) & 0xFFFF;
    uint16_t seconds = (timestamp / 1000) % 60;
    snprintf(buffer, bufferSize, "%02d:%02d", minutes, seconds);
}

const char* SaveSystem::getLastErrorString() const {
    if (!m_lastError) return "No error";
    return "Save operation failed";
}