// ============================================================================
// NEON REQUIEM — Dialog System (Header)
// ============================================================================
// Система диалогов. Загружает диалоги из JSON с SD-карты, управляет
// typewriter-эффектом, выбором реплик, ветвлением диалогов по маршрутам.
// Поддерживает внутренние монологи Кая (отдельный стиль).
// ============================================================================

#ifndef NEON_REQUIEM_DIALOG_SYSTEM_H
#define NEON_REQUIEM_DIALOG_SYSTEM_H

#include <cstdint>
#include <Arduino.h>
#include <ArduinoJson.h>
#include "config/config.h"
#include "config/palette.h"
#include "../../engine/scene_manager.h"

// ============================================================================
// ТИПЫ ДИАЛОГОВ
// ============================================================================

/// Кто говорит
enum class Speaker : uint8_t {
    NARRATOR  = 0,  /// Рассказчик (без портрета)
    KAI       = 1,  /// Главный герой
    ECHO      = 2,  /// Призрак-напарник
    RAZOR     = 3,  /// Инфо-брокер
    LYRA      = 4,  /// Хирург
    ZERO      = 5,  /// Ребёнок-ИИ
    CORTANA   = 6,  /// Босс арки 1
    COLLECTOR = 7,  /// Босс арки 2
    SIREN     = 8,  /// Босс арки 3
    KANE      = 9,  /// Главный антагонист
    SYSTEM    = 10, /// Системное сообщение
    MONOLOGUE = 11  /// Внутренний монолог Кая
};

/// Тип диалогового узла
enum class DialogNodeType : uint8_t {
    SPEECH    = 0,  /// Обычная речь
    CHOICE    = 1,  /// Выбор реплики
    CONDITION = 2,  /// Условный переход (по маршруту/квесту)
    ACTION    = 3,  /// Действие (запуск боя, катсцены)
    END       = 4   /// Конец диалога
};

/// Вариант выбора
struct DialogChoice {
    char     text[DIALOG_MAX_LINE_LENGTH];  /// Текст выбора
    uint16_t nextNodeId;                     /// ID следующего узла
    int8_t   routeRequirement;               /// Требуемый маршрут (-1 = любой)
    int8_t   questRequirement;               /// Требуемый квест (-1 = любой)
};

/// Узел диалога
struct DialogNode {
    DialogNodeType type;                       /// Тип узла
    Speaker        speaker;                    /// Говорящий
    char           text[DIALOG_MAX_LINE_LENGTH * 3]; /// Текст (до 3 строк)
    uint16_t       nextNodeId;                 /// ID следующего узла (для SPEECH)
    uint8_t        choiceCount;                /// Количество вариантов
    DialogChoice   choices[DIALOG_MAX_CHOICES];/// Варианты выбора
    uint16_t       actionParam;                /// Параметр действия (ID боя/катсцены)
};

// ============================================================================
// DIALOG SCENE (наследует Scene)
// ============================================================================

class DialogScene : public Scene {
public:
    DialogScene();
    virtual ~DialogScene() = default;

    /// Загрузить диалог из JSON
    /// @param dialogId — ID диалога (соответствует файлу на SD)
    bool loadDialog(uint16_t dialogId);

    /// Начать диалог с указанного узла
    void start(uint16_t startNodeId = 0);

    /// Обновление (typewriter, ввод)
    virtual void update() override;

    /// Отрисовка
    virtual void render() override;

    /// При входе в сцену
    virtual void onEnter() override;

    /// При выходе
    virtual void onExit() override;

    /// Проверить, активен ли диалог
    bool isDialogActive() const { return m_active; }

    /// Получить текущего говорящего
    Speaker getCurrentSpeaker() const { return m_currentSpeaker; }

    /// Установить callback на завершение диалога
    void setOnEndCallback(void (*callback)()) { m_onEndCallback = callback; }

private:
    /// Загрузить узлы из JSON
    bool parseNodes(const JsonDocument& doc);

    /// Перейти к следующему узлу
    void advanceToNode(uint16_t nodeId);

    /// Обработать выбор
    void selectChoice(uint8_t index);

    /// Обновить typewriter-эффект
    void updateTypewriter();

    /// Получить цвет для говорящего
    uint16_t getSpeakerColor(Speaker speaker) const;

    /// Получить имя говорящего
    const char* getSpeakerName(Speaker speaker) const;

    /// Узлы диалога
    DialogNode* m_nodes;
    uint16_t    m_nodeCount;
    uint16_t    m_currentNodeId;

    /// Текущий говорящий
    Speaker m_currentSpeaker;

    /// Typewriter
    char     m_displayedText[DIALOG_MAX_LINE_LENGTH * 3];
    uint16_t m_textLength;       /// Сколько символов уже показано
    uint16_t m_textTargetLength; /// Сколько всего символов
    uint32_t m_typewriterTimer;

    /// Выбор
    int8_t m_selectedChoice;  /// -1 = нет выбора, 0-3 = индекс

    /// Состояние
    bool m_dialogLoaded;
    bool m_waitingForInput;  /// Ожидание нажатия A для продолжения

    /// Callback
    void (*m_onEndCallback)();

    /// Флаг инициализации
    bool m_initialized;
};

#endif // NEON_REQUIEM_DIALOG_SYSTEM_H