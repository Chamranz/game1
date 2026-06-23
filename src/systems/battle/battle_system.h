// ============================================================================
// NEON REQUIEM — Battle System (Header)
// ============================================================================
// Undertale-style боевая система. Верхняя часть экрана — меню действий
// и противник. Нижняя — SOUL BOX (зона уклонения от пуль).
// 4 действия: HACK, SCAN, LINK, MERCY.
// 2 типа боёв: REACTION (уклонение) и PATTERN (запоминание паттернов).
// ============================================================================

#ifndef NEON_REQUIEM_BATTLE_SYSTEM_H
#define NEON_REQUIEM_BATTLE_SYSTEM_H

#include <cstdint>
#include <Arduino.h>
#include "config/config.h"
#include "config/palette.h"
#include "../../engine/scene_manager.h"

// ============================================================================
// ТИПЫ БОЁВ
// ============================================================================

/// Тип боя
enum class BattleType : uint8_t {
    NORMAL    = 0,  /// Обычный враг
    BOSS      = 1,  /// Босс (много фаз)
    MINIBOSS  = 2   /// Мини-босс
};

/// Тип атаки
enum class AttackType : uint8_t {
    REACTION  = 0,  /// Уклонение от пуль (bullet-hell)
    PATTERN   = 1   /// Запоминание и повторение паттерна
};

/// Действие игрока
enum class BattleAction : uint8_t {
    HACK   = 0,  /// Атака (наносит урон)
    SCAN   = 1,  /// Сканирование (узнаёт слабости)
    LINK   = 2,  /// Связь (восстанавливает HP/Resonance)
    MERCY  = 3,  /// Пощада (завершает бой)
    NONE   = 4
};

/// Фаза боя
enum class BattlePhase : uint8_t {
    INTRO     = 0,  /// Вступительный диалог
    PLAYER    = 1,  /// Ход игрока (выбор действия)
    ENEMY     = 2,  /// Ход врага (bullet-hell)
    RESULT    = 3,  /// Результат хода
    VICTORY   = 4,  /// Победа
    DEFEAT    = 5,  /// Поражение
    COUNTER   = 6   /// COUNTER-HACK (парирование)
};

// ============================================================================
// ВРАГ
// ============================================================================

struct EnemyData {
    char        name[24];       /// Имя врага
    int         maxHP;          /// Максимальное HP
    int         hp;             /// Текущее HP
    int         attack;         /// Сила атаки
    int         defense;        /// Защита
    uint16_t    color;          /// Цвет спрайта
    uint8_t     phaseCount;     /// Количество фаз (для боссов)
    uint8_t     currentPhase;   /// Текущая фаза
    AttackType  attackType;     /// Тип атаки
    uint16_t    dialogId;       /// ID диалога для этого врага
};

// ============================================================================
// BATTLE SCENE
// ============================================================================

class BattleScene : public Scene {
public:
    BattleScene();
    virtual ~BattleScene() = default;

    /// Инициализация боя
    /// @param enemyData — данные врага
    /// @param battleType — тип боя
    void startBattle(const EnemyData& enemyData, BattleType battleType);

    /// Обновление
    virtual void update() override;

    /// Отрисовка
    virtual void render() override;

    /// При входе
    virtual void onEnter() override;

    /// При выходе
    virtual void onExit() override;

    /// Проверить, активен ли бой
    bool isBattleActive() const { return m_active; }

    /// Установить callback на победу
    void setOnVictoryCallback(void (*callback)()) { m_onVictory = callback; }

    /// Установить callback на поражение
    void setOnDefeatCallback(void (*callback)()) { m_onDefeat = callback; }

private:
    /// Обновление фазы INTRO
    void updateIntro();

    /// Обновление фазы PLAYER (выбор действия)
    void updatePlayerTurn();

    /// Обновление фазы ENEMY (bullet-hell)
    void updateEnemyTurn();

    /// Обновление фазы RESULT
    void updateResult();

    /// Обновление фазы COUNTER-HACK
    void updateCounterHack();

    /// Выполнить действие
    void executeAction(BattleAction action);

    /// Атака HACK
    void doHack();

    /// Сканирование SCAN
    void doScan();

    /// Связь LINK
    void doLink();

    /// Пощада MERCY
    void doMercy();

    /// Переключение фазы босса
    void checkPhaseTransition();

    /// Проверка победы/поражения
    void checkBattleEnd();

    /// Отрисовка верхней части (меню + враг)
    void renderTop();

    /// Отрисовка нижней части (SOUL BOX)
    void renderBottom();

    /// Отрисовка меню действий
    void renderActionMenu();

    /// Отрисовка HP бара врага
    void renderEnemyHP();

    /// Отрисовка HP игрока
    void renderPlayerHP();

    // Данные врага
    EnemyData   m_enemy;
    BattleType  m_battleType;

    // Состояние боя
    BattlePhase m_phase;
    BattleAction m_selectedAction;
    BattleAction m_lastAction;
    int         m_turnCount;

    // HP игрока в бою
    int         m_playerBattleHP;
    int         m_playerMaxBattleHP;

    // SCAN данные
    bool        m_scanned;
    int         m_scanLevel;

    // COUNTER-HACK
    bool        m_counterHackAvailable;
    uint32_t    m_counterHackTimer;

    // Анимация
    int         m_animTimer;
    int         m_animFrame;

    // Callback'и
    void (*m_onVictory)();
    void (*m_onDefeat)();

    // Флаг инициализации
    bool m_initialized;
};

#endif // NEON_REQUIEM_BATTLE_SYSTEM_H