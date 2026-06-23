// ============================================================================
// NEON REQUIEM — SD Card Manager (Implementation)
// ============================================================================
// Реализация работы с SD-картой. Чтение файлов, JSON-парсинг, кэширование
// в PSRAM для быстрого доступа к спрайтам и диалогам.
// ============================================================================

#include "sd_manager.h"

// ============================================================================
// СИНГЛТОН
// ============================================================================

SDManager& SDManager::getInstance() {
    static SDManager instance;
    return instance;
}

// ============================================================================
// КОНСТРУКТОР
// ============================================================================

SDManager::SDManager()
    : m_initialized(false)
    , m_ready(false)
{
    for (int i = 0; i < CACHE_SIZE; i++) {
        m_cache[i].path[0] = '\0';
        m_cache[i].data = nullptr;
        m_cache[i].size = 0;
        m_cache[i].used = false;
    }
}

// ============================================================================
// ИНИЦИАЛИЗАЦИЯ
// ============================================================================

bool SDManager::begin() {
    LOG_INFO("SDManager: initializing SD card...");

    // Настраиваем пины SPI для SD
    SPI.begin(PIN_SD_SCLK, PIN_SD_MISO, PIN_SD_MOSI, PIN_SD_CS);

    // Пытаемся инициализировать SD
    uint32_t startTime = millis();
    bool sdInit = false;

    while ((millis() - startTime) < SD_INIT_TIMEOUT_MS) {
        if (SD.begin(PIN_SD_CS, SPI, SD_SPI_FREQ)) {
            sdInit = true;
            break;
        }
        delay(100);
    }

    if (!sdInit) {
        LOG_ERROR("SDManager: SD card initialization failed!");
        LOG_ERROR("SDManager: check card connection and format (FAT32)");
        m_ready = false;
        m_initialized = true;
        return false;
    }

    // Проверяем тип файловой системы
    uint8_t cardType = SD.cardType();
    if (cardType == CARD_NONE) {
        LOG_ERROR("SDManager: no SD card attached");
        m_ready = false;
        m_initialized = true;
        return false;
    }

    // Проверяем наличие необходимых директорий
    const char* requiredDirs[] = {
        SD_PATH_SPRITES,
        SD_PATH_MUSIC,
        SD_PATH_SFX,
        SD_PATH_MAPS,
        SD_PATH_DIALOGS,
        SD_PATH_FONTS,
        SD_PATH_SAVES
    };

    for (const char* dir : requiredDirs) {
        if (!SD.exists(dir)) {
            LOG_WARN_F("SDManager: directory %s not found, creating...", dir);
            SD.mkdir(dir);
        }
    }

    // Выводим информацию о SD
    uint64_t totalBytes = SD.totalBytes();
    uint64_t usedBytes  = SD.usedBytes();

    LOG_INFO_F("SDManager: initialized. Type: %d, Total: %llu MB, Used: %llu MB",
               cardType, totalBytes / (1024 * 1024), usedBytes / (1024 * 1024));

    m_ready = true;
    m_initialized = true;
    return true;
}

// ============================================================================
// ПРОВЕРКА СОСТОЯНИЯ
// ============================================================================

bool SDManager::isReady() const {
    return m_ready;
}

uint64_t SDManager::getTotalSpace() const {
    return m_ready ? SD.totalBytes() : 0;
}

uint64_t SDManager::getFreeSpace() const {
    return m_ready ? (SD.totalBytes() - SD.usedBytes()) : 0;
}

// ============================================================================
// ЧТЕНИЕ ФАЙЛОВ
// ============================================================================

uint8_t* SDManager::readFile(const char* path, size_t& size) {
    if (!m_ready) {
        LOG_ERROR("SDManager: SD not ready");
        size = 0;
        return nullptr;
    }

    File file = SD.open(path, FILE_READ);
    if (!file) {
        LOG_ERROR_F("SDManager: cannot open file: %s", path);
        size = 0;
        return nullptr;
    }

    size = file.size();
    if (size == 0) {
        LOG_WARN_F("SDManager: empty file: %s", path);
        file.close();
        return nullptr;
    }

    // Выделяем буфер
    uint8_t* buffer = (uint8_t*)malloc(size);
    if (!buffer) {
        LOG_ERROR("SDManager: memory allocation failed");
        file.close();
        size = 0;
        return nullptr;
    }

    // Читаем файл
    size_t bytesRead = file.read(buffer, size);
    file.close();

    if (bytesRead != size) {
        LOG_ERROR_F("SDManager: read %d of %d bytes from %s", bytesRead, size, path);
        free(buffer);
        size = 0;
        return nullptr;
    }

    LOG_VERBOSE_F("SDManager: loaded %s (%d bytes)", path, size);
    return buffer;
}

uint8_t* SDManager::readFileToPSRAM(const char* path, size_t& size) {
    if (!m_ready) {
        LOG_ERROR("SDManager: SD not ready");
        size = 0;
        return nullptr;
    }

    File file = SD.open(path, FILE_READ);
    if (!file) {
        LOG_ERROR_F("SDManager: cannot open file: %s", path);
        size = 0;
        return nullptr;
    }

    size = file.size();
    if (size == 0) {
        file.close();
        return nullptr;
    }

    // Выделяем буфер в PSRAM
    uint8_t* buffer = (uint8_t*)ps_malloc(size);
    if (!buffer) {
        LOG_ERROR("SDManager: PSRAM allocation failed");
        file.close();
        size = 0;
        return nullptr;
    }

    size_t bytesRead = file.read(buffer, size);
    file.close();

    if (bytesRead != size) {
        LOG_ERROR_F("SDManager: read %d of %d bytes from %s", bytesRead, size, path);
        free(buffer);
        size = 0;
        return nullptr;
    }

    LOG_VERBOSE_F("SDManager: loaded %s to PSRAM (%d bytes)", path, size);
    return buffer;
}

// ============================================================================
// ЧТЕНИЕ JSON
// ============================================================================

bool SDManager::readJSON(const char* path, JsonDocument& doc) {
    if (!m_ready) {
        LOG_ERROR("SDManager: SD not ready");
        return false;
    }

    File file = SD.open(path, FILE_READ);
    if (!file) {
        LOG_ERROR_F("SDManager: cannot open JSON: %s", path);
        return false;
    }

    // Парсим JSON
    DeserializationError error = deserializeJson(doc, file);
    file.close();

    if (error) {
        LOG_ERROR_F("SDManager: JSON parse error in %s: %s", path, error.c_str());
        return false;
    }

    LOG_VERBOSE_F("SDManager: loaded JSON: %s", path);
    return true;
}

// ============================================================================
// ПРОВЕРКА СУЩЕСТВОВАНИЯ
// ============================================================================

bool SDManager::fileExists(const char* path) {
    if (!m_ready) return false;
    return SD.exists(path);
}

size_t SDManager::getFileSize(const char* path) {
    if (!m_ready) return 0;

    File file = SD.open(path, FILE_READ);
    if (!file) return 0;

    size_t size = file.size();
    file.close();
    return size;
}

// ============================================================================
// НАВИГАЦИЯ
// ============================================================================

int SDManager::listDirectory(const char* path, SDFileInfo* files, int maxFiles) {
    if (!m_ready || !files) return 0;

    File dir = SD.open(path);
    if (!dir || !dir.isDirectory()) {
        LOG_ERROR_F("SDManager: cannot open directory: %s", path);
        return 0;
    }

    int count = 0;
    File entry = dir.openNextFile();

    while (entry && count < maxFiles) {
        strncpy(files[count].name, entry.name(), sizeof(files[count].name) - 1);
        strncpy(files[count].path, path, sizeof(files[count].path) - 1);

        // Добавляем имя файла к пути
        size_t pathLen = strlen(files[count].path);
        if (pathLen > 0 && files[count].path[pathLen - 1] != '/') {
            strncat(files[count].path, "/", sizeof(files[count].path) - pathLen - 1);
        }
        strncat(files[count].path, entry.name(),
                sizeof(files[count].path) - strlen(files[count].path) - 1);

        files[count].size = entry.size();
        files[count].isDirectory = entry.isDirectory();

        count++;
        entry = dir.openNextFile();
    }

    entry.close();
    dir.close();

    return count;
}

void SDManager::buildPath(const char* base, const char* name, const char* ext, char* out) {
    strcpy(out, base);
    strcat(out, name);
    strcat(out, ext);
}

// ============================================================================
// КЭШИРОВАНИЕ
// ============================================================================

int SDManager::cacheFile(const char* path) {
    if (!m_ready) return -1;

    // Ищем свободный слот кэша
    int slot = -1;
    for (int i = 0; i < CACHE_SIZE; i++) {
        if (!m_cache[i].used) {
            slot = i;
            break;
        }
    }

    if (slot < 0) {
        // Кэш полон — вытесняем первый
        slot = 0;
        if (m_cache[slot].data) {
            free(m_cache[slot].data);
            m_cache[slot].data = nullptr;
        }
    }

    // Загружаем файл в PSRAM
    size_t size = 0;
    uint8_t* data = readFileToPSRAM(path, size);
    if (!data) return -1;

    // Сохраняем в кэш
    strncpy(m_cache[slot].path, path, sizeof(m_cache[slot].path) - 1);
    m_cache[slot].data = data;
    m_cache[slot].size = size;
    m_cache[slot].used = true;

    LOG_VERBOSE_F("SDManager: cached %s (slot %d)", path, slot);
    return slot;
}

uint8_t* SDManager::getCachedData(int cacheId, size_t& size) {
    if (cacheId < 0 || cacheId >= CACHE_SIZE || !m_cache[cacheId].used) {
        size = 0;
        return nullptr;
    }

    size = m_cache[cacheId].size;
    return m_cache[cacheId].data;
}

void SDManager::clearCache() {
    for (int i = 0; i < CACHE_SIZE; i++) {
        if (m_cache[i].data) {
            free(m_cache[i].data);
            m_cache[i].data = nullptr;
        }
        m_cache[i].path[0] = '\0';
        m_cache[i].size = 0;
        m_cache[i].used = false;
    }
    LOG_INFO("SDManager: cache cleared");
}

// ============================================================================
// ВАЛИДАЦИЯ
// ============================================================================

bool SDManager::validateWAV(const uint8_t* data, size_t size) {
    if (!data || size < 44) return false;
    return memcmp(data, "RIFF", 4) == 0 && memcmp(data + 8, "WAVE", 4) == 0;
}

bool SDManager::validateSPR(const uint8_t* data, size_t size) {
    if (!data || size < 8) return false;
    // SPR-формат: первые 4 байта — магическое число "NRSP"
    return memcmp(data, "NRSP", 4) == 0;
}