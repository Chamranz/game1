// ============================================================================
// NEON REQUIEM — Audio Manager (Header)
// ============================================================================
// Управление звуком через I2S MAX98357A. Поддерживает:
// - Проигрывание WAV-файлов с SD-карты
// - Микширование до 4 каналов одновременно
// - Потоковое воспроизведение музыки из PSRAM
// - Звуковые эффекты (SFX) с приоритетами
// - Регулировка громкости
// ============================================================================

#ifndef NEON_REQUIEM_AUDIO_MANAGER_H
#define NEON_REQUIEM_AUDIO_MANAGER_H

#include <cstdint>
#include <Arduino.h>
#include "config/config.h"
#include "config/pins.h"

// ============================================================================
// ТИПЫ АУДИО
// ============================================================================

/// Тип аудио-ресурса
enum class AudioType : uint8_t {
    MUSIC = 0,  /// Фоновая музыка
    SFX   = 1,  /// Звуковой эффект
    VOICE = 2   /// Голос (диалоги)
};

/// Приоритет звука (для SFX)
enum class AudioPriority : uint8_t {
    PRIORITY_LOW     = 0,
    NORMAL  = 1,
    PRIORITY_HIGH    = 2,
    CRITICAL = 3
};

/// Состояние аудио-канала
enum class AudioChannelState : uint8_t {
    STOPPED  = 0,
    PLAYING  = 1,
    PAUSED   = 2,
    FADING   = 3
};

/// Структура WAV-заголовка (44 байта)
struct WAVHeader {
    char     riff[4];         // "RIFF"
    uint32_t fileSize;        // Размер файла - 8
    char     wave[4];         // "WAVE"
    char     fmt[4];          // "fmt "
    uint32_t fmtSize;         // 16
    uint16_t audioFormat;     // 1 = PCM
    uint16_t numChannels;     // 1=моно, 2=стерео
    uint32_t sampleRate;      // Частота дискретизации
    uint32_t byteRate;        // Байт в секунду
    uint16_t blockAlign;      // Выравнивание
    uint16_t bitsPerSample;   // Бит на сэмпл
    char     data[4];         // "data"
    uint32_t dataSize;        // Размер данных
} __attribute__((packed));

/// Аудио-канал
struct AudioChannel {
    AudioChannelState state;     /// Состояние
    AudioType         type;      /// Тип
    AudioPriority     priority;  /// Приоритет
    uint8_t*          data;      /// Указатель на WAV-данные в PSRAM
    uint32_t          dataSize;  /// Размер данных
    uint32_t          position;  /// Текущая позиция воспроизведения (байты)
    uint16_t          volume;    /// Громкость канала (0-255)
    bool              loop;      /// Зациклить?
    float             fadeStep;  /// Шаг затухания (для FADING)
    uint32_t          fadeTarget;/// Целевая громкость при затухании
};

// ============================================================================
// AUDIO MANAGER
// ============================================================================

class AudioManager {
public:
    /// Получить единственный экземпляр (синглтон)
    static AudioManager& getInstance();

    /// Инициализация I2S
    /// @return true если успешно
    bool begin();

    /// Обновление аудио-микшера (вызывать каждый кадр)
    void update();

    /// Загрузить WAV-файл с SD-карты в PSRAM
    /// @param path — путь к файлу на SD
    /// @param type — тип аудио
    /// @return ID канала (0-3) или -1 если ошибка
    int8_t loadWAV(const char* path, AudioType type);

    /// Воспроизвести звук
    /// @param channelId — ID канала (0-3)
    /// @param loop — зациклить?
    /// @param priority — приоритет
    /// @return true если успешно
    bool play(int8_t channelId, bool loop = false,
              AudioPriority priority = AudioPriority::NORMAL);

    /// Остановить канал
    void stop(int8_t channelId);

    /// Остановить все каналы
    void stopAll();

    /// Поставить на паузу
    void pause(int8_t channelId);

    /// Возобновить
    void resume(int8_t channelId);

    /// Плавное затухание (fade out)
    /// @param channelId — ID канала
    /// @param durationMs — длительность затухания в мс
    void fadeOut(int8_t channelId, uint32_t durationMs);

    /// Установить громкость канала
    void setVolume(int8_t channelId, uint16_t volume);

    /// Получить громкость канала
    uint16_t getVolume(int8_t channelId) const;

    /// Установить глобальную громкость
    void setMasterVolume(uint16_t volume);

    /// Получить глобальную громкость
    uint16_t getMasterVolume() const;

    /// Проверить, играет ли канал
    bool isPlaying(int8_t channelId) const;

    /// Выгрузить WAV из памяти
    void unload(int8_t channelId);

    /// Выгрузить все
    void unloadAll();

private:
    AudioManager();
    ~AudioManager() = default;
    AudioManager(const AudioManager&) = delete;
    AudioManager& operator=(const AudioManager&) = delete;

    /// Инициализация I2S-интерфейса
    bool initI2S();

    /// Микширование каналов в выходной буфер
    void mixChannels();

    /// Запись сэмпла в I2S (DMA)
    void writeSample(int16_t sample);

    /// Чтение WAV-заголовка
    bool parseWAVHeader(const uint8_t* data, WAVHeader& header);

    /// Поиск свободного канала
    int8_t findFreeChannel(AudioPriority priority);

    /// Прерывание I2S (DMA)
    static void onI2STxDone();

    // Аудио-каналы
    AudioChannel m_channels[AUDIO_MAX_SIMULTANEOUS];

    // Глобальная громкость
    uint16_t m_masterVolume;

    // I2S
    bool m_i2sInitialized;

    // DMA буфер
    int16_t* m_dmaBuffer;
    size_t   m_dmaBufferSize;

    // Флаг инициализации
    bool m_initialized;
};

#endif // NEON_REQUIEM_AUDIO_MANAGER_H