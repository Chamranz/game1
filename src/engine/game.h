// ============================================================================
// NEON REQUIEM — Game Engine (Header)
// ============================================================================
// Главный класс игры. Управляет жизненным циклом:
// - Инициализация всех систем
// - Главный игровой цикл (30 FPS)
// - Переключение между состояниями (overworld, battle, dialog, cutscene)
// - Обработка ошибок и восстановление
// ============================================================================

#ifndef NEON_REQUIEM_GAME_H
#define NEON_REQUIEM_GAME_H

#include <cstdint>
#include <Arduino.h>
#include "config/config.h"
#include "config/palette.h"
#include "../core/input/input_manager.h"
#include "../core/display/display_manager.h"
#include "../core/audio/audio_manager.h"
#include "../core/storage/sd_manager.h"

// ============================================================================
// ПРОТОТИПЫ СИСТЕМ (определены позже)
// ============================================================================

class SceneManager;
class DialogScene;
class BattleScene;
class CutsceneScene;
class OverworldScene;
class ResonanceSystem;
class QuestManager;
class RouteManager;
class SaveSystem;
class Inventory;
class HUD;
class SpriteRenderer;
class TilemapRenderer;

// ============================================================================
// GAME CLASS
// ============================================================================

class Game {
public:
    /// Получить единственный экземпляр (синглтон)
    static Game& getInstance();

    /// Запуск игры
    void run();

    /// Переключение состояния игры
    void setState(GameState newState);

    /// Получить текущее состояние
    GameState getState() const;

    /// Получить предыдущее состояние
    GameState getPreviousState() const;

    /// Установить маршрут
    void setRoute(GameRoute route);

    /// Получить текущий маршрут
    GameRoute getRoute() const;

    /// Получить номер текущей арки (1-5)
    uint8_t getCurrentArc() const;

    /// Установить текущую арку
    void setCurrentArc(uint8_t arc);

    /// Завершить игру
    void shutdown();

    /// Перезагрузить игру
    void restart();

    // ========================================================================
    // ДОСТУП К СИСТЕМАМ
    // ========================================================================

    InputManager&     getInput()     { return InputManager::getInstance(); }
    DisplayManager&   getDisplay()   { return DisplayManager::getInstance(); }
    AudioManager&     getAudio()     { return AudioManager::getInstance(); }
    SDManager&        getStorage()   { return SDManager::getInstance(); }

    SceneManager*     getSceneManager()   const { return m_sceneManager; }
    DialogScene*      getDialogSystem()   const { return m_dialogScene; }
    BattleScene*      getBattleSystem()   const { return m_battleScene; }
    CutsceneScene*    getCutsceneSystem() const { return m_cutsceneScene; }
    OverworldScene*   getOverworld()      const { return m_overworld; }
    ResonanceSystem*  getResonance()      const { return m_resonance; }
    QuestManager*     getQuestManager()   const { return m_questManager; }
    RouteManager*     getRouteManager()   const { return m_routeManager; }
    SaveSystem*       getSaveSystem()     const { return m_saveSystem; }
    Inventory*        getInventory()      const { return m_inventory; }
    HUD*              getHUD()            const { return m_hud; }
    SpriteRenderer*   getSpriteRenderer() const { return m_spriteRenderer; }
    TilemapRenderer*  getTilemapRenderer()const { return m_tilemapRenderer; }

    /// Получить FPS
    float getFPS() const { return m_fps; }

    /// Получить использование памяти
    size_t getFreeHeap() const;
    size_t getFreePSRAM() const;

private:
    Game();
    ~Game() = default;
    Game(const Game&) = delete;
    Game& operator=(const Game&) = delete;

    /// Инициализация всех систем
    bool init();

    /// Инициализация подсистем (поэтапно)
    bool initCore();      // config, input, display, audio, sd
    bool initEngine();    // scene manager, player
    bool initSystems();   // dialog, battle, cutscene, overworld, etc.
    bool initRendering(); // sprite renderer, tilemap, hud, effects
    bool initContent();   // загрузка контента первой арки

    /// Главный игровой цикл
    void gameLoop();

    /// Обновление логики
    void update();

    /// Отрисовка кадра
    void render();

    /// Обработка ошибок
    void handleError(const char* errorMsg);

    /// Показать экран загрузки
    void showBootScreen();

    /// Показать экран ошибки
    void showErrorScreen(const char* errorMsg);

    /// Синхронизация FPS
    void syncFPS();

    // ========================================================================
    // СИСТЕМЫ (инициализируются в init*())
    // ========================================================================

    // Engine
    SceneManager*    m_sceneManager;

    // Systems
    DialogScene*     m_dialogScene;
    BattleScene*     m_battleScene;
    CutsceneScene*   m_cutsceneScene;
    OverworldScene*  m_overworld;
    ResonanceSystem* m_resonance;
    QuestManager*    m_questManager;
    RouteManager*    m_routeManager;
    SaveSystem*      m_saveSystem;
    Inventory*       m_inventory;

    // Rendering
    HUD*             m_hud;
    SpriteRenderer*  m_spriteRenderer;
    TilemapRenderer* m_tilemapRenderer;

    // ========================================================================
    // СОСТОЯНИЕ
    // ========================================================================

    GameState  m_state;
    GameState  m_previousState;
    GameRoute  m_route;
    uint8_t    m_currentArc;

    // FPS
    float      m_fps;
    uint32_t   m_lastFrameTime;
    uint32_t   m_frameCount;
    uint32_t   m_fpsTimer;

    // Флаги
    bool m_initialized;
    bool m_running;
    bool m_error;
};

#endif // NEON_REQUIEM_GAME_H