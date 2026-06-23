// ============================================================================
// NEON REQUIEM — Audio Manager (Implementation)
// ============================================================================
// Реализация аудиосистемы через I2S. WAV-файлы загружаются с SD-карты
// в PSRAM, микшируются в реальном времени и выводятся через MAX98357A.
// ============================================================================

#include "audio_manager.h"
#include <SD.h>
#include <driver/i2s.h>

// ============================================================================
// КОНФИГУРАЦИЯ I2S
// ============================================================================

/// Конфигурация I2S для MAX98357A
#define I2S_PORT             I2S_NUM_0
#define I2S_DMA_BUF_COUNT    8
#define I2S_DMA_BUF_LEN      256

// ============================================================================
// СИНГЛТОН
// ============================================================================

AudioManager& AudioManager::getInstance() {
    static AudioManager instance;
    return instance;
}

// ============================================================================
// КОНСТРУКТОР
// ============================================================================

AudioManager::AudioManager()
    : m_masterVolume(AUDIO_DEFAULT_VOLUME)
    , m_i2sInitialized(false)
    , m_dmaBuffer(nullptr)
    , m_dmaBufferSize(0)
    , m_initialized(false)
{
    // Обнуляем каналы
    for (int i = 0; i < AUDIO_MAX_SIMULTANEOUS; i++) {
        m_channels[i].state    = AudioChannelState::STOPPED;
        m_channels[i].type     = AudioType::SFX;
        m_channels[i].priority = AudioPriority::PRIORITY_LOW;
        m_channels[i].data     = nullptr;
        m_channels[i].dataSize = 0;
        m_channels[i].position = 0;
        m_channels[i].volume   = 255;
        m_channels[i].loop     = false;
        m_channels[i].fadeStep = 0.0f;
        m_channels[i].fadeTarget = 0;
    }
}

// ============================================================================
// ИНИЦИАЛИЗАЦИЯ
// ============================================================================

bool AudioManager::begin() {
    LOG_INFO("AudioManager: initializing I2S...");

    if (!initI2S()) {
        LOG_ERROR("AudioManager: I2S init failed!");
        return false;
    }

    // Выделяем DMA буфер
    m_dmaBufferSize = I2S_DMA_BUF_COUNT * I2S_DMA_BUF_LEN;
    m_dmaBuffer = (int16_t*)heap_caps_malloc(
        m_dmaBufferSize * sizeof(int16_t),
        MALLOC_CAP_DMA
    );

    if (!m_dmaBuffer) {
        LOG_ERROR("AudioManager: DMA buffer allocation failed!");
        return false;
    }

    m_initialized = true;
    LOG_INFO("AudioManager: initialized successfully");
    return true;
}

// ============================================================================
// ИНИЦИАЛИЗАЦИЯ I2S
// ============================================================================

bool AudioManager::initI2S() {
    i2s_config_t i2sConfig = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
        .sample_rate = AUDIO_SAMPLE_RATE,
        .bits_per_sample = (i2s_bits_per_sample_t)AUDIO_BITS,
        .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = I2S_DMA_BUF_COUNT,
        .dma_buf_len = I2S_DMA_BUF_LEN,
        .use_apll = true,
        .tx_desc_auto_clear = true,
        .fixed_mclk = 0
    };

    i2s_pin_config_t pinConfig = {
        .bck_io_num = PIN_I2S_BCLK,
        .ws_io_num = PIN_I2S_LRC,
        .data_out_num = PIN_I2S_DOUT,
        .data_in_num = I2S_PIN_NO_CHANGE
    };

    esp_err_t err = i2s_driver_install(I2S_PORT, &i2sConfig, 0, NULL);
    if (err != ESP_OK) {
        LOG_ERROR_F("AudioManager: I2S install failed: %d", err);
        return false;
    }

    err = i2s_set_pin(I2S_PORT, &pinConfig);
    if (err != ESP_OK) {
        LOG_ERROR_F("AudioManager: I2S set pin failed: %d", err);
        return false;
    }

    // Устанавливаем частоту дискретизации
    i2s_set_sample_rates(I2S_PORT, AUDIO_SAMPLE_RATE);

    m_i2sInitialized = true;
    return true;
}

// ============================================================================
// ЗАГРУЗКА WAV
// ============================================================================

int8_t AudioManager::loadWAV(const char* path, AudioType type) {
    LOG_INFO_F("AudioManager: loading WAV: %s", path);

    // Открываем файл на SD
    File file = SD.open(path);
    if (!file) {
        LOG_ERROR_F("AudioManager: cannot open file: %s", path);
        return -1;
    }

    // Читаем WAV-заголовок
    WAVHeader header;
    if (file.read((uint8_t*)&header, sizeof(WAVHeader)) != sizeof(WAVHeader)) {
        LOG_ERROR("AudioManager: failed to read WAV header");
        file.close();
        return -1;
    }

    // Проверяем WAV
    if (memcmp(header.riff, "RIFF", 4) != 0 || memcmp(header.wave, "WAVE", 4) != 0) {
        LOG_ERROR("AudioManager: invalid WAV file");
        file.close();
        return -1;
    }

    // Проверяем формат (PCM, 16-bit)
    if (header.audioFormat != 1 || header.bitsPerSample != 16) {
        LOG_ERROR("AudioManager: unsupported WAV format (only PCM 16-bit)");
        file.close();
        return -1;
    }

    // Выделяем память в PSRAM для аудиоданных
    uint8_t* audioData = (uint8_t*)ps_malloc(header.dataSize);
    if (!audioData) {
        LOG_ERROR("AudioManager: PSRAM allocation failed for audio data");
        file.close();
        return -1;
    }

    // Читаем аудиоданные
    size_t bytesRead = file.read(audioData, header.dataSize);
    if (bytesRead != header.dataSize) {
        LOG_ERROR("AudioManager: incomplete WAV read");
        free(audioData);
        file.close();
        return -1;
    }

    file.close();

    // Ищем свободный канал
    int8_t channelId = findFreeChannel(AudioPriority::NORMAL);
    if (channelId < 0) {
        LOG_ERROR("AudioManager: no free channels");
        free(audioData);
        return -1;
    }

    // Настраиваем канал
    m_channels[channelId].data     = audioData;
    m_channels[channelId].dataSize = header.dataSize;
    m_channels[channelId].type     = type;
    m_channels[channelId].state    = AudioChannelState::STOPPED;
    m_channels[channelId].position = 0;
    m_channels[channelId].volume   = 255;
    m_channels[channelId].loop     = false;

    LOG_INFO_F("AudioManager: loaded %s (%d bytes, ch=%d)", path, header.dataSize, channelId);
    return channelId;
}

// ============================================================================
// ВОСПРОИЗВЕДЕНИЕ
// ============================================================================

bool AudioManager::play(int8_t channelId, bool loop, AudioPriority priority) {
    if (channelId < 0 || channelId >= AUDIO_MAX_SIMULTANEOUS) return false;
    if (!m_channels[channelId].data) return false;

    m_channels[channelId].state    = AudioChannelState::PLAYING;
    m_channels[channelId].position = 0;
    m_channels[channelId].loop     = loop;
    m_channels[channelId].priority = priority;

    LOG_INFO_F("AudioManager: playing channel %d (loop=%d)", channelId, loop);
    return true;
}

// ============================================================================
// ОСТАНОВКА
// ============================================================================

void AudioManager::stop(int8_t channelId) {
    if (channelId < 0 || channelId >= AUDIO_MAX_SIMULTANEOUS) return;
    m_channels[channelId].state = AudioChannelState::STOPPED;
    m_channels[channelId].position = 0;
}

void AudioManager::stopAll() {
    for (int i = 0; i < AUDIO_MAX_SIMULTANEOUS; i++) {
        stop(i);
    }
}

// ============================================================================
// ПАУЗА / ВОЗОБНОВЛЕНИЕ
// ============================================================================

void AudioManager::pause(int8_t channelId) {
    if (channelId < 0 || channelId >= AUDIO_MAX_SIMULTANEOUS) return;
    if (m_channels[channelId].state == AudioChannelState::PLAYING) {
        m_channels[channelId].state = AudioChannelState::PAUSED;
    }
}

void AudioManager::resume(int8_t channelId) {
    if (channelId < 0 || channelId >= AUDIO_MAX_SIMULTANEOUS) return;
    if (m_channels[channelId].state == AudioChannelState::PAUSED) {
        m_channels[channelId].state = AudioChannelState::PLAYING;
    }
}

// ============================================================================
// ЗАТУХАНИЕ
// ============================================================================

void AudioManager::fadeOut(int8_t channelId, uint32_t durationMs) {
    if (channelId < 0 || channelId >= AUDIO_MAX_SIMULTANEOUS) return;
    if (m_channels[channelId].state != AudioChannelState::PLAYING) return;

    m_channels[channelId].state = AudioChannelState::FADING;
    m_channels[channelId].fadeTarget = 0;
    m_channels[channelId].fadeStep = (float)m_channels[channelId].volume /
                                     (durationMs / (1000.0f / TARGET_FPS));
}

// ============================================================================
// ГРОМКОСТЬ
// ============================================================================

void AudioManager::setVolume(int8_t channelId, uint16_t volume) {
    if (channelId < 0 || channelId >= AUDIO_MAX_SIMULTANEOUS) return;
    m_channels[channelId].volume = volume;
}

uint16_t AudioManager::getVolume(int8_t channelId) const {
    if (channelId < 0 || channelId >= AUDIO_MAX_SIMULTANEOUS) return 0;
    return m_channels[channelId].volume;
}

void AudioManager::setMasterVolume(uint16_t volume) {
    m_masterVolume = volume;
}

uint16_t AudioManager::getMasterVolume() const {
    return m_masterVolume;
}

// ============================================================================
// ПРОВЕРКА ВОСПРОИЗВЕДЕНИЯ
// ============================================================================

bool AudioManager::isPlaying(int8_t channelId) const {
    if (channelId < 0 || channelId >= AUDIO_MAX_SIMULTANEOUS) return false;
    return m_channels[channelId].state == AudioChannelState::PLAYING ||
           m_channels[channelId].state == AudioChannelState::FADING;
}

// ============================================================================
// ВЫГРУЗКА
// ============================================================================

void AudioManager::unload(int8_t channelId) {
    if (channelId < 0 || channelId >= AUDIO_MAX_SIMULTANEOUS) return;
    stop(channelId);
    if (m_channels[channelId].data) {
        free(m_channels[channelId].data);
        m_channels[channelId].data = nullptr;
    }
}

void AudioManager::unloadAll() {
    for (int i = 0; i < AUDIO_MAX_SIMULTANEOUS; i++) {
        unload(i);
    }
}

// ============================================================================
// ОБНОВЛЕНИЕ (МИКШЕР)
// ============================================================================

void AudioManager::update() {
    if (!m_initialized) return;

    mixChannels();
}

void AudioManager::mixChannels() {
    // Размер выходного буфера (стерео сэмплы)
    size_t bufSize = I2S_DMA_BUF_LEN;
    int16_t mixBuffer[bufSize * 2];  // Стерео

    // Обнуляем микшер
    memset(mixBuffer, 0, sizeof(mixBuffer));

    // Микшируем все активные каналы
    for (int ch = 0; ch < AUDIO_MAX_SIMULTANEOUS; ch++) {
        if (m_channels[ch].state != AudioChannelState::PLAYING &&
            m_channels[ch].state != AudioChannelState::FADING) {
            continue;
        }

        if (!m_channels[ch].data) continue;

        // Обработка затухания
        if (m_channels[ch].state == AudioChannelState::FADING) {
            if (m_channels[ch].volume <= m_channels[ch].fadeStep) {
                m_channels[ch].volume = 0;
                m_channels[ch].state = AudioChannelState::STOPPED;
                continue;
            }
            m_channels[ch].volume -= m_channels[ch].fadeStep;
        }

        // Микшируем сэмплы
        for (size_t i = 0; i < bufSize; i++) {
            if (m_channels[ch].position >= m_channels[ch].dataSize) {
                if (m_channels[ch].loop) {
                    m_channels[ch].position = 0;
                } else {
                    m_channels[ch].state = AudioChannelState::STOPPED;
                    break;
                }
            }

            // Читаем 16-bit сэмпл (моно)
            int16_t sample = (m_channels[ch].data[m_channels[ch].position + 1] << 8) |
                              m_channels[ch].data[m_channels[ch].position];
            m_channels[ch].position += 2;

            // Применяем громкость
            sample = (sample * m_channels[ch].volume * m_masterVolume) / (255 * 255);

            // Микшируем в стерео (моно -> оба канала)
            mixBuffer[i * 2]     += sample;
            mixBuffer[i * 2 + 1] += sample;
        }
    }

    // Отправляем в I2S
    size_t bytesWritten = 0;
    i2s_write(I2S_PORT, mixBuffer, bufSize * 2 * sizeof(int16_t), &bytesWritten, portMAX_DELAY);
}

// ============================================================================
// ЗАПИСЬ СЭМПЛА В I2S
// ============================================================================

void AudioManager::writeSample(int16_t sample) {
    // Не используется напрямую — микшер пишет через i2s_write
}

// ============================================================================
// ПАРСИНГ WAV
// ============================================================================

bool AudioManager::parseWAVHeader(const uint8_t* data, WAVHeader& header) {
    if (!data) return false;
    memcpy(&header, data, sizeof(WAVHeader));
    return memcmp(header.riff, "RIFF", 4) == 0 && memcmp(header.wave, "WAVE", 4) == 0;
}

// ============================================================================
// ПОИСК СВОБОДНОГО КАНАЛА
// ============================================================================

int8_t AudioManager::findFreeChannel(AudioPriority priority) {
    // Сначала ищем полностью свободный
    for (int i = 0; i < AUDIO_MAX_SIMULTANEOUS; i++) {
        if (m_channels[i].state == AudioChannelState::STOPPED) {
            return i;
        }
    }

    // Если нет свободных — вытесняем канал с самым низким приоритетом
    int8_t lowestPriChannel = -1;
    AudioPriority lowestPri = AudioPriority::CRITICAL;

    for (int i = 0; i < AUDIO_MAX_SIMULTANEOUS; i++) {
        if (m_channels[i].priority < lowestPri) {
            lowestPri = m_channels[i].priority;
            lowestPriChannel = i;
        }
    }

    // Вытесняем только если наш приоритет выше
    if (lowestPriChannel >= 0 && priority > lowestPri) {
        stop(lowestPriChannel);
        if (m_channels[lowestPriChannel].data) {
            free(m_channels[lowestPriChannel].data);
            m_channels[lowestPriChannel].data = nullptr;
        }
        return lowestPriChannel;
    }

    return -1;  // Нет свободных каналов
}

// ============================================================================
// ПРЕРЫВАНИЕ I2S
// ============================================================================

void AudioManager::onI2STxDone() {
    // Обработка DMA прерывания (зарезервировано для будущего использования)
}