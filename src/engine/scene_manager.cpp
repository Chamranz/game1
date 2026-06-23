// ============================================================================
// NEON REQUIEM — Scene Manager (Implementation)
// ============================================================================
// Реализация стека сцен. Поддерживает наложение сцен друг на друга
// (например, диалог поверх overworld) и управление переходами.
// ============================================================================

#include "scene_manager.h"

// ============================================================================
// КОНСТРУКТОР / ДЕСТРУКТОР
// ============================================================================

SceneManager::SceneManager()
    : m_stackDepth(0)
    , m_initialized(false)
{
    for (int i = 0; i < MAX_SCENE_STACK; i++) {
        m_stack[i] = nullptr;
    }
}

SceneManager::~SceneManager() {
    clearAll();
}

// ============================================================================
// ИНИЦИАЛИЗАЦИЯ
// ============================================================================

bool SceneManager::begin() {
    m_initialized = true;
    LOG_INFO("SceneManager: initialized");
    return true;
}

// ============================================================================
// ОБНОВЛЕНИЕ
// ============================================================================

void SceneManager::update() {
    if (m_stackDepth <= 0) return;

    // Обновляем верхнюю сцену
    Scene* top = m_stack[m_stackDepth - 1];
    if (top && top->isActive()) {
        top->update();
    }
}

// ============================================================================
// ОТРИСОВКА
// ============================================================================

void SceneManager::render() {
    if (m_stackDepth <= 0) return;

    // Рисуем все сцены снизу вверх, пока не встретим непрозрачную
    int startLayer = m_stackDepth - 1;

    for (int i = m_stackDepth - 1; i >= 0; i--) {
        if (m_stack[i] && !m_stack[i]->hasFlag(SCENE_TRANSPARENT)) {
            startLayer = i;
            break;
        }
    }

    // Рендерим с самой нижней непрозрачной сцены
    for (int i = startLayer; i < m_stackDepth; i++) {
        if (m_stack[i] && m_stack[i]->isActive()) {
            m_stack[i]->render();
        }
    }
}

// ============================================================================
// УПРАВЛЕНИЕ СТЕКОМ
// ============================================================================

void SceneManager::switchTo(Scene* scene) {
    if (!scene) return;

    // Выходим из всех текущих сцен
    clearAll();

    // Входим в новую сцену
    m_stack[0] = scene;
    m_stackDepth = 1;
    scene->onEnter();

    LOG_INFO_F("SceneManager: switched to scene type %d", static_cast<int>(scene->getType()));
}

void SceneManager::pushScene(Scene* scene) {
    if (!scene) return;
    if (m_stackDepth >= MAX_SCENE_STACK) {
        LOG_WARN("SceneManager: scene stack overflow!");
        return;
    }

    // Паузим текущую сцену
    if (m_stackDepth > 0 && m_stack[m_stackDepth - 1]) {
        m_stack[m_stackDepth - 1]->onPause();
    }

    // Добавляем новую
    m_stack[m_stackDepth] = scene;
    m_stackDepth++;
    scene->onEnter();

    LOG_INFO_F("SceneManager: pushed scene type %d (depth: %d)",
               static_cast<int>(scene->getType()), m_stackDepth);
}

void SceneManager::popScene() {
    if (m_stackDepth <= 0) return;

    // Выходим из верхней сцены
    m_stackDepth--;
    if (m_stack[m_stackDepth]) {
        m_stack[m_stackDepth]->onExit();
        // Не удаляем — владелец сам управляет памятью
        m_stack[m_stackDepth] = nullptr;
    }

    // Возобновляем предыдущую
    if (m_stackDepth > 0 && m_stack[m_stackDepth - 1]) {
        m_stack[m_stackDepth - 1]->onResume();
    }

    LOG_INFO_F("SceneManager: popped scene (depth: %d)", m_stackDepth);
}

void SceneManager::replaceTop(Scene* scene) {
    if (!scene || m_stackDepth <= 0) return;

    // Выходим из текущей верхней
    if (m_stack[m_stackDepth - 1]) {
        m_stack[m_stackDepth - 1]->onExit();
    }

    // Заменяем
    m_stack[m_stackDepth - 1] = scene;
    scene->onEnter();

    LOG_INFO_F("SceneManager: replaced top with scene type %d",
               static_cast<int>(scene->getType()));
}

// ============================================================================
// ДОСТУП
// ============================================================================

Scene* SceneManager::getCurrentScene() const {
    if (m_stackDepth <= 0) return nullptr;
    return m_stack[m_stackDepth - 1];
}

Scene* SceneManager::findScene(SceneType type) const {
    for (int i = m_stackDepth - 1; i >= 0; i--) {
        if (m_stack[i] && m_stack[i]->getType() == type) {
            return m_stack[i];
        }
    }
    return nullptr;
}

void SceneManager::clearAll() {
    for (int i = 0; i < m_stackDepth; i++) {
        if (m_stack[i]) {
            m_stack[i]->onExit();
            m_stack[i] = nullptr;
        }
    }
    m_stackDepth = 0;
    LOG_INFO("SceneManager: cleared all scenes");
}

int SceneManager::getSceneCount() const {
    return m_stackDepth;
}