// ============================================================================
// NEON REQUIEM — Cutscene System (Header)
// ============================================================================
// Система катсцен. Управляет проигрыванием анимированных сцен с
// параллакс-эффектами, субтитрами, музыкой и эффектами (дождь, глитч).
// ============================================================================

#ifndef NEON_REQUIEM_CUTSCENE_SYSTEM_H
#define NEON_REQUIEM_CUTSCENE_SYSTEM_H

#include <cstdint>
#include <Arduino.h>
#include "config/config.h"
#include "config/palette.h"
#include "../../engine/scene_manager.h"

/// Максимум слоёв в катсцене
constexpr int CUTSCENE_MAX_LAYERS = 4;

/// Тип слоя катсцены
enum class CutsceneLayerType : uint8_t {
    BACKGROUND = 0,  /// Фоновый слой
    SPRITE     = 1,  /// Спрайт персонажа
    EFFECT     = 2,  /// Эффект (дождь, глитч)
    TEXT       = 3   /// Текст/субтитры
};

/// Кадр катсцены
struct CutsceneFrame {
    uint16_t duration;          /// Длительность кадра (мс)
    uint16_t bgColor;           /// Цвет фона
    int8_t   layerCount;        /// Количество слоёв
    uint16_t dialogText[3];     /// Текст субтитров (до 3 строк)
    uint8_t  effectType;        /// Тип эффекта (0=нет, 1=дождь, 2=глитч, 3=хром. аберрация)
    uint8_t  effectIntensity;   /// Интенсивность эффекта (0-255)
};

/// Сцена катсцены
struct CutsceneData {
    uint16_t frameCount;        /// Количество кадров
    uint16_t musicId;           /// ID музыки
    bool     skippable;         /// Можно пропустить
    CutsceneFrame* frames;      /// Массив кадров
};

class CutsceneScene : public Scene {
public:
    CutsceneScene();
    virtual ~CutsceneScene();

    /// Загрузить катсцену
    bool loadCutscene(uint16_t cutsceneId);

    /// Начать проигрывание
    void play();

    /// Обновление
    virtual void update() override;

    /// Отрисовка
    virtual void render() override;

    virtual void onEnter() override;
    virtual void onExit() override;

    /// Проверить, проигрывается ли
    bool isPlaying() const { return m_playing; }

    /// Установить callback на завершение
    void setOnEndCallback(void (*callback)()) { m_onEnd = callback; }

private:
    CutsceneData m_data;
    int m_currentFrame;
    uint32_t m_frameTimer;
    bool m_playing;
    bool m_skipped;
    void (*m_onEnd)();
};

#endif