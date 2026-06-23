// ============================================================================
// NEON REQUIEM — SD Card Manager (Header)
// ============================================================================
// Управление SD-картой. Обеспечивает:
// - Инициализацию SD-карты в SPI-режиме
// - Чтение JSON-файлов (диалоги, карты, конфиги)
// - Чтение бинарных спрайтов (.spr)
// - Чтение WAV-файлов
// - Кэширование часто используемых файлов в PSRAM
// - Навигацию по директориям
// ============================================================================

#ifndef NEON_REQUIEM_SD_MANAGER_H
#define NEON_REQUIEM_SD_MANAGER_H

#include <cstdint>
#include <Arduino.h>
#include <SD.h>
#include <FS.h>
#include <ArduinoJson.h>
#include "config/config.h"
#include "config/pins.h"

// ============================================================================
// SD FILE INFO
// ============================================================================

/// Информация о файле на SD
struct SDFileInfo {
    char     name[64];       /// Имя файла
    char     path[128];      /// Полный путь
    uint32_t size;           /// Размер в байтах
    bool     isDirectory;    /// Это директория?
};

// ============================================================================
// SD MANAGER
// ============================================================================

class SDManager {
public:
    /// Получить единственный экземпляр (синглтон)
    static SDManager& getInstance();

    /// Инициализация SD-карты
    /// @return true если успешно
    bool begin();

    /// Проверить, инициализирована ли SD
    bool isReady() const;

    /// Получить общий размер SD (в байтах)
    uint64_t getTotalSpace() const;

    /// Получить свободное место (в байтах)
    uint64_t getFreeSpace() const;

    // ========================================================================
    // ЧТЕНИЕ ФАЙЛОВ
    // ========================================================================

    /// Прочитать весь файл в буфер (heap)
    /// @param path — путь к файлу
    /// @param size — [out] размер прочитанных данных
    /// @return указатель на буфер или nullptr
    uint8_t* readFile(const char* path, size_t& size);

    /// Прочитать файл в PSRAM
    /// @param path — путь к файлу
    /// @param size — [out] размер прочитанных данных
    /// @return указатель на буфер в PSRAM или nullptr
    uint8_t* readFileToPSRAM(const char* path, size_t& size);

    /// Прочитать JSON-файл
    /// @param path — путь к JSON
    /// @param doc — [out] ссылка на JsonDocument
    /// @return true если успешно
    bool readJSON(const char* path, JsonDocument& doc);

    /// Проверить существование файла
    bool fileExists(const char* path);

    /// Получить размер файла
    size_t getFileSize(const char* path);

    // ========================================================================
    // НАВИГАЦИЯ
    // ========================================================================

    /// Открыть директорию
    /// @param path — путь к директории
    /// @param files — [out] массив для результатов
    /// @param maxFiles — максимум файлов
    /// @return количество найденных файлов
    int listDirectory(const char* path, SDFileInfo* files, int maxFiles);

    /// Получить полный путь к ресурсу
    /// @param base — базовая директория (SD_PATH_*)
    /// @param name — имя файла
    /// @param ext — расширение
    /// @param out — [out] буфер для результата
    void buildPath(const char* base, const char* name, const char* ext, char* out);

    // ========================================================================
    // КЭШИРОВАНИЕ
    // ========================================================================

    /// Загрузить файл в кэш PSRAM
    /// @param path — путь к файлу
    /// @return ID кэша или -1
    int cacheFile(const char* path);

    /// Получить данные из кэша по ID
    /// @param cacheId — ID кэша
    /// @param size — [out] размер данных
    /// @return указатель на данные или nullptr
    uint8_t* getCachedData(int cacheId, size_t& size);

    /// Очистить кэш
    void clearCache();

private:
    SDManager();
    ~SDManager() = default;
    SDManager(const SDManager&) = delete;
    SDManager& operator=(const SDManager&) = delete;

    /// Проверка WAV-файла
    bool validateWAV(const uint8_t* data, size_t size);

    /// Проверка SPR-файла
    bool validateSPR(const uint8_t* data, size_t size);

    // Структура кэша
    struct CacheEntry {
        char     path[128];
        uint8_t* data;
        size_t   size;
        bool     used;
    };

    // Кэш (маленький — только для часто используемых файлов)
    static constexpr int CACHE_SIZE = 8;
    CacheEntry m_cache[CACHE_SIZE];

    // Состояние
    bool m_initialized;
    bool m_ready;
};

#endif // NEON_REQUIEM_SD_MANAGER_H