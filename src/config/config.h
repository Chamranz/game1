// ============================================================================
// NEON REQUIEM — Master Configuration Header
// ============================================================================
// Этот файл содержит все глобальные константы игры: версию, размеры экрана,
// настройки памяти, тайминги, лимиты систем и переключатели фич.
// Все макросы — constexpr для оптимизации на этапе компиляции.
// ============================================================================

#ifndef NEON_REQUIEM_CONFIG_H
#define NEON_REQUIEM_CONFIG_H

#include <cstdint>

// ============================================================================
// ВЕРСИЯ И ИДЕНТИФИКАЦИЯ
// ============================================================================

/// Версия игры (semver)
#define GAME_VERSION_MAJOR    1
#define GAME_VERSION_MINOR    0
#define GAME_VERSION_PATCH    0

/// Строковое представление версии
#define GAME_VERSION_STR      "1.0.0"

/// Название игры
#define GAME_TITLE            "NEON REQUIEM"
#define GAME_SUBTITLE         "A Cyberpunk Noir RPG"

/// Авторские права
#define GAME_COPYRIGHT        "(c) 2026 NEON REQUIEM Team"

// ============================================================================
// ДИСПЛЕЙ
// ============================================================================

/// Разрешение дисплея (TFT ST7789 2.0" 320x240)
constexpr int DISPLAY_WIDTH      = 320;
constexpr int DISPLAY_HEIGHT     = 240;

/// Глубина цвета: 16-bit RGB565
constexpr int DISPLAY_BPP        = 16;

/// Частота обновления дисплея (SPI)
constexpr int DISPLAY_SPI_FREQ   = 80000000;  // 80 MHz

/// Размер аппаратного буфера (в пикселях)
constexpr int DISPLAY_BUFFER_SIZE = DISPLAY_WIDTH * DISPLAY_HEIGHT;

// ============================================================================
// ТАЙЛОВАЯ КАРТА
// ============================================================================

/// Размер одного тайла в пикселях
constexpr int TILE_SIZE          = 16;

/// Размер карты в тайлах
constexpr int MAP_WIDTH_TILES    = 10;
constexpr int MAP_HEIGHT_TILES   = 8;

/// Размер карты в пикселях
constexpr int MAP_WIDTH_PX       = MAP_WIDTH_TILES * TILE_SIZE;   // 640
constexpr int MAP_HEIGHT_PX      = MAP_HEIGHT_TILES * TILE_SIZE;  // 480

/// Количество слоёв карты (0=фон, 1=объекты, 2=верхний)
constexpr int MAP_LAYERS         = 3;

/// Максимум тайлов в тайлсете
constexpr int TILESET_MAX_TILES  = 128;  // было 256

// ============================================================================
// ПАМЯТЬ (ESP32-S3 N16R8: 16MB Flash + 8MB PSRAM)
// ============================================================================

/// Размер PSRAM
constexpr size_t PSRAM_SIZE      = 8 * 1024 * 1024;  // 8 MB

/// Лимиты PSRAM для разных подсистем
constexpr size_t PSRAM_SPRITES   = 3 * 1024 * 1024;  // 3 MB — спрайты
constexpr size_t PSRAM_AUDIO     = 2 * 1024 * 1024;  // 2 MB — аудио
constexpr size_t PSRAM_MAPS      = 1 * 1024 * 1024;  // 1 MB — карты
constexpr size_t PSRAM_DIALOGS   = 512 * 1024;       // 512 KB — диалоги
constexpr size_t PSRAM_UI        = 256 * 1024;       // 256 KB — UI элементы
constexpr size_t PSRAM_EFFECTS   = 512 * 1024;       // 512 KB — эффекты
constexpr size_t PSRAM_RESERVE   = 720 * 1024;       // ~720 KB — резерв

/// Флаг: использовать PSRAM для спрайтов (если доступна)
#define USE_PSRAM_SPRITES       1
#define USE_PSRAM_AUDIO         1
#define USE_PSRAM_MAPS          1

// ============================================================================
// ПРОИЗВОДИТЕЛЬНОСТЬ
// ============================================================================

/// Целевой FPS
constexpr int TARGET_FPS         = 30;

/// Длительность кадра в мс
constexpr int FRAME_TIME_MS      = 1000 / TARGET_FPS;  // ~33 мс

constexpr int MAX_SPRITES        = 16;   // было 64
constexpr int MAX_PARTICLES      = 32;   // было 128
constexpr int MAX_NPCS           = 4;    // было 16
constexpr int MAX_MAP_OBJECTS    = 8;    // было 32

// ============================================================================
// ИГРОВЫЕ СИСТЕМЫ
// ============================================================================

/// Максимальное количество HP игрока
constexpr int PLAYER_MAX_HP      = 20;

/// Максимальное количество HP босса
constexpr int BOSS_MAX_HP        = 99;

/// Скорость движения игрока (пикселей/кадр)
constexpr float PLAYER_SPEED     = 2.0f;

/// Радиус взаимодействия с NPC (в пикселях)
constexpr int INTERACT_RADIUS    = 20;

/// Длительность typewriter-эффекта (мс на символ)
constexpr int TYPEWRITER_DELAY_MS = 35;

/// Максимальная длина строки диалога
constexpr int DIALOG_MAX_LINE_LENGTH = 80;

/// Максимум вариантов выбора в диалоге
constexpr int DIALOG_MAX_CHOICES = 4;

/// Размер SOUL BOX (зона уклонения в бою)
constexpr int SOUL_BOX_WIDTH     = 80;
constexpr int SOUL_BOX_HEIGHT    = 60;

/// Скорость пули в bullet-hell (пикселей/кадр)
constexpr float BULLET_SPEED     = 3.0f;

/// Максимум пуль на экране в бою
constexpr int MAX_BULLETS        = 8;    // было 32

/// Длительность COUNTER-HACK окна (мс)
constexpr int COUNTER_HACK_WINDOW_MS = 200;

// ============================================================================
// СЮЖЕТНЫЕ ЛИМИТЫ
// ============================================================================

/// Всего арок
constexpr int TOTAL_ARCS         = 5;

/// Всего маршрутов
constexpr int TOTAL_ROUTES       = 3;

/// Маршруты
enum class GameRoute : uint8_t {
    RESONANCE = 0,  /// Тёплый маршрут — искупление
    ERASE     = 1,  /// Агрессивный маршрут — власть
    GLITCH    = 2   /// Ломаный маршрут — истина
};

/// Состояния игры
enum class GameState : uint8_t {
    BOOT          = 0,   /// Загрузка
    TITLE         = 1,   /// Экран заголовка
    OVERWORLD     = 2,   /// Свободное перемещение
    DIALOG        = 3,   /// Диалог
    BATTLE        = 4,   /// Бой
    CUTSCENE      = 5,   /// Катсцена
    MENU          = 6,   /// Меню
    RESONANCE     = 7,   /// Режим RESONANCE
    CINEMATIC     = 8,   /// Заставка
    GAME_OVER     = 9,   /// Поражение
    VICTORY       = 10,  /// Победа
    CREDITS       = 11   /// Титры
};

/// Максимум квестов одновременно
constexpr int MAX_ACTIVE_QUESTS  = 3;    // было 8

/// Максимум предметов в инвентаре
constexpr int MAX_INVENTORY      = 16;   // было 32

// ============================================================================
// АУДИО
// ============================================================================

/// Частота дискретизации аудио
constexpr int AUDIO_SAMPLE_RATE  = 44100;

/// Битрейт аудио
constexpr int AUDIO_BITS         = 16;

/// Количество аудиоканалов
constexpr int AUDIO_CHANNELS     = 2;  // Стерео

/// Громкость по умолчанию (0-255)
constexpr uint8_t AUDIO_DEFAULT_VOLUME = 200;

/// Максимум одновременно проигрываемых звуков
constexpr int AUDIO_MAX_SIMULTANEOUS = 4;

// ============================================================================
// SD-КАРТА
// ============================================================================

/// Пути к директориям на SD-карте
#define SD_PATH_SPRITES     "/sprites/"
#define SD_PATH_MUSIC       "/music/"
#define SD_PATH_SFX         "/sfx/"
#define SD_PATH_MAPS        "/maps/"
#define SD_PATH_DIALOGS     "/dialogs/"
#define SD_PATH_FONTS       "/fonts/"
#define SD_PATH_SAVES       "/saves/"

/// Расширения файлов
#define EXT_SPRITE          ".spr"
#define EXT_MAP             ".json"
#define EXT_DIALOG          ".json"
#define EXT_SAVE            ".sav"
#define EXT_WAV             ".wav"

// ============================================================================
// ОТЛАДКА
// ============================================================================

/// Уровень отладки (0=выкл, 1=ошибки, 2=предупреждения, 3=инфо, 4=verbose)
#define DEBUG_LEVEL         3

/// Макросы отладки
// Variadic versions: LOG_ERROR("fmt %d", arg) — uses printf-style formatting
// Single-arg versions: LOG_ERROR("message") — also works via printf
#if DEBUG_LEVEL >= 1
    #define LOG_ERROR(fmt, ...)      Serial.printf("[ERROR] " fmt "\n", ##__VA_ARGS__)
    #define LOG_ERROR_F(fmt, ...)    Serial.printf("[ERROR] " fmt "\n", ##__VA_ARGS__)
#else
    #define LOG_ERROR(fmt, ...)
    #define LOG_ERROR_F(fmt, ...)
#endif

#if DEBUG_LEVEL >= 2
    #define LOG_WARN(fmt, ...)       Serial.printf("[WARN] " fmt "\n", ##__VA_ARGS__)
    #define LOG_WARN_F(fmt, ...)     Serial.printf("[WARN] " fmt "\n", ##__VA_ARGS__)
#else
    #define LOG_WARN(fmt, ...)
    #define LOG_WARN_F(fmt, ...)
#endif

#if DEBUG_LEVEL >= 3
    #define LOG_INFO(fmt, ...)       Serial.printf("[INFO] " fmt "\n", ##__VA_ARGS__)
    #define LOG_INFO_F(fmt, ...)     Serial.printf("[INFO] " fmt "\n", ##__VA_ARGS__)
#else
    #define LOG_INFO(fmt, ...)
    #define LOG_INFO_F(fmt, ...)
#endif

#if DEBUG_LEVEL >= 4
    #define LOG_VERBOSE(fmt, ...)    Serial.printf("[VERB] " fmt "\n", ##__VA_ARGS__)
    #define LOG_VERBOSE_F(fmt, ...)  Serial.printf("[VERB] " fmt "\n", ##__VA_ARGS__)
#else
    #define LOG_VERBOSE(fmt, ...)
    #define LOG_VERBOSE_F(fmt, ...)
#endif

/// Включить FPS-счётчик
#define DEBUG_SHOW_FPS      1

/// Включить профайлинг памяти
#define DEBUG_MEMORY        1

// ============================================================================
// ФИЧИ (можно отключать для экономии памяти)
// ============================================================================

/// Включить систему RESONANCE
#define FEATURE_RESONANCE   1

/// Включить стелс-механику
#define FEATURE_STEALTH     1

/// Включить систему сохранений
#define FEATURE_SAVE        1

/// Включить систему инвентаря
#define FEATURE_INVENTORY   1

/// Включить систему квестов
#define FEATURE_QUESTS      1

/// Включить эффекты (дождь, глитч, CRT)
#define FEATURE_EFFECTS     1

/// Включить параллакс в катсценах
#define FEATURE_PARALLAX    1

/// Включить музыку
#define FEATURE_MUSIC       1

/// Включить звуковые эффекты
#define FEATURE_SFX         1

// ============================================================================
// ТАЙМАУТЫ
// ============================================================================

/// Таймаут инициализации SD (мс)
constexpr int SD_INIT_TIMEOUT_MS     = 5000;

/// Таймаут загрузки спрайта (мс)
constexpr int SPRITE_LOAD_TIMEOUT_MS = 3000;

/// Таймаут загрузки аудио (мс)
constexpr int AUDIO_LOAD_TIMEOUT_MS  = 5000;

/// Задержка автоповтора кнопок (мс)
constexpr int BUTTON_REPEAT_DELAY_MS = 200;

/// Задержка первого повторения кнопки (мс)
constexpr int BUTTON_REPEAT_INIT_MS  = 400;

// ============================================================================
// ВСПОМОГАТЕЛЬНЫЕ МАКРОСЫ
// ============================================================================

/// Преобразование миллисекунд в кадры (при 30 FPS)
#define MS_TO_FRAMES(ms)     ((ms) * TARGET_FPS / 1000)

/// Преобразование кадров в миллисекунды
#define FRAMES_TO_MS(frames) ((frames) * 1000 / TARGET_FPS)

/// Количество элементов в массиве
#define ARRAY_SIZE(arr)      (sizeof(arr) / sizeof((arr)[0]))

/// Минимум и максимум
#define CLAMP(val, min, max) (((val) < (min)) ? (min) : (((val) > (max)) ? (max) : (val)))

/// Проверка флага
#define FLAG_IS_SET(val, flag) (((val) & (flag)) == (flag))

#endif // NEON_REQUIEM_CONFIG_H