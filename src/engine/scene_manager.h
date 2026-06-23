// ============================================================================
// NEON REQUIEM — Scene Manager (Header)
// ============================================================================
// Управление сценами и переходами между ними. Каждая сцена — это
// самостоятельная единица с собственным update() и render().
// Поддерживает стек сцен для наложения (например, диалог поверх overworld).
// ============================================================================

#ifndef NEON_REQUIEM_SCENE_MANAGER_H
#define NEON_REQUIEM_SCENE_MANAGER_H

#include <cstdint>
#include <Arduino.h>
#include "config/config.h"

// ============================================================================
// ТИПЫ СЦЕН
// ============================================================================

/// Тип сцены
enum class SceneType : uint8_t {
    NONE        = 0,
    OVERWORLD   = 1,  /// Свободное перемещение
    BATTLE      = 2,  /// Бой
    DIALOG      = 3,  /// Диалог
    CUTSCENE    = 4,  /// Катсцена
    MENU        = 5,  /// Меню
    TITLE       = 6,  /// Титульный экран
    CINEMATIC   = 7,  /// Заставка
    RESONANCE   = 8,  /// Режим RESONANCE
    MINIGAME    = 9   /// Мини-игра
};

/// Флаги сцены
enum SceneFlags : uint8_t {
    SCENE_NONE       = 0,
    SCENE_TRANSPARENT = 1 << 0,  /// Сцена прозрачна (видно ниже)
    SCENE_PAUSABLE   = 1 << 1,   /// Можно поставить на паузу
    SCENE_SKIPPABLE  = 1 << 2,   /// Можно пропустить (катсцены)
    SCENE_NO_INPUT   = 1 << 3    /// Не принимает ввод
};

// ============================================================================
// БАЗОВЫЙ КЛАСС СЦЕНЫ
// ============================================================================

class Scene {
public:
    Scene(SceneType type) : m_type(type), m_flags(SCENE_NONE), m_active(false) {}
    virtual ~Scene() = default;

    /// Обновление логики сцены
    virtual void update() = 0;

    /// Отрисовка сцены
    virtual void render() = 0;

    /// Вызывается при входе в сцену
    virtual void onEnter() { m_active = true; }

    /// Вызывается при выходе из сцены
    virtual void onExit()  { m_active = false; }

    /// Вызывается при наложении другой сцены
    virtual void onPause() {}

    /// Вызывается при возврате из наложенной сцены
    virtual void onResume() {}

    /// Получить тип сцены
    SceneType getType() const { return m_type; }

    /// Проверить флаг
    bool hasFlag(SceneFlags flag) const { return (m_flags & flag) != 0; }

    /// Установить флаг
    void setFlag(SceneFlags flag) { m_flags |= flag; }

    /// Снять флаг
    void clearFlag(SceneFlags flag) { m_flags &= ~flag; }

    /// Активна ли сцена
    bool isActive() const { return m_active; }

protected:
    SceneType  m_type;
    uint8_t    m_flags;
    bool       m_active;
};

// ============================================================================
// SCENE MANAGER
// ============================================================================

class SceneManager {
public:
    SceneManager();
    ~SceneManager();

    /// Инициализация
    bool begin();

    /// Обновление активной сцены
    void update();

    /// Отрисовка активной сцены
    void render();

    /// Переключиться на сцену (заменяет текущую)
    void switchTo(Scene* scene);

    /// Наложить сцену поверх текущей
    void pushScene(Scene* scene);

    /// Убрать верхнюю сцену
    void popScene();

    /// Заменить верхнюю сцену
    void replaceTop(Scene* scene);

    /// Получить текущую сцену
    Scene* getCurrentScene() const;

    /// Получить сцену по типу (ищет сверху вниз)
    Scene* findScene(SceneType type) const;

    /// Очистить все сцены
    void clearAll();

    /// Количество сцен в стеке
    int getSceneCount() const;

private:
    /// Максимальная глубина стека сцен
    static constexpr int MAX_SCENE_STACK = 8;

    /// Стек сцен
    Scene* m_stack[MAX_SCENE_STACK];
    int    m_stackDepth;

    /// Флаг инициализации
    bool m_initialized;
};

#endif // NEON_REQUIEM_SCENE_MANAGER_H