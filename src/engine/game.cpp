// ============================================================================
// NEON REQUIEM — Game Engine (Implementation)
// ============================================================================
// Main game loop. Initializes all systems, manages states,
// synchronizes FPS and handles errors.
// ============================================================================

#include "game.h"
#include "scene_manager.h"
#include "../systems/dialog/dialog_system.h"
#include "../systems/battle/battle_system.h"
#include "../systems/cutscene/cutscene_system.h"
#include "../systems/overworld/overworld_scene.h"
#include "../systems/resonance/resonance_system.h"
#include "../systems/quest/quest_manager.h"
#include "../systems/route/route_manager.h"
#include "../systems/save/save_system.h"
#include "../systems/inventory/inventory.h"
#include "../rendering/sprite_renderer.h"
#include "../rendering/tilemap_renderer.h"
#include "../rendering/ui/hud.h"
#include "../rendering/ui/menu_ui.h"
#include "../rendering/effects/rain_effect.h"
#include "../rendering/effects/glitch_effect.h"
#include "../rendering/effects/crt_effect.h"
#include "../rendering/transitions/fade_transition.h"
#include "../rendering/animation/sprite_animation.h"
#include "../content/content_manager.h"

// ============================================================================
// SINGLETON
// ============================================================================

Game& Game::getInstance() {
    static Game instance;
    return instance;
}

// ============================================================================
// CONSTRUCTOR
// ============================================================================

Game::Game()
    : m_sceneManager(nullptr)
    , m_dialogScene(nullptr)
    , m_battleScene(nullptr)
    , m_cutsceneScene(nullptr)
    , m_overworld(nullptr)
    , m_resonance(nullptr)
    , m_questManager(nullptr)
    , m_routeManager(nullptr)
    , m_saveSystem(nullptr)
    , m_inventory(nullptr)
    , m_hud(nullptr)
    , m_spriteRenderer(nullptr)
    , m_tilemapRenderer(nullptr)
    , m_state(GameState::BOOT)
    , m_previousState(GameState::BOOT)
    , m_route(GameRoute::RESONANCE)
    , m_currentArc(1)
    , m_fps(0.0f)
    , m_lastFrameTime(0)
    , m_frameCount(0)
    , m_fpsTimer(0)
    , m_initialized(false)
    , m_running(false)
    , m_error(false)
{
}

// ============================================================================
// RUN
// ============================================================================

void Game::run() {
    Serial.begin(115200);
    delay(100);
    Serial.println();
    Serial.println("========================================");
    Serial.printf("  %s v%s\n", GAME_TITLE, GAME_VERSION_STR);
    Serial.println("========================================");

    showBootScreen();

    if (!init()) {
        handleError("Failed to initialize game systems");
        return;
    }

    m_running = true;
    m_lastFrameTime = millis();
    m_fpsTimer = millis();

    LOG_INFO("Game: entering main loop");
    gameLoop();
}

// ============================================================================
// INIT
// ============================================================================

bool Game::init() {
    LOG_INFO("Game: initializing...");

    if (!initCore())     { LOG_ERROR("Game: core init failed");     return false; }
    if (!initEngine())   { LOG_ERROR("Game: engine init failed");   return false; }
    if (!initSystems())  { LOG_ERROR("Game: systems init failed");  return false; }
    if (!initRendering()){ LOG_ERROR("Game: rendering init failed");return false; }
    if (!initContent())  { LOG_ERROR("Game: content init failed");  return false; }

    m_initialized = true;
    m_state = GameState::TITLE;

    LOG_INFO("Game: initialization complete");
    return true;
}

// ============================================================================
// CORE INIT
// ============================================================================

bool Game::initCore() {
    LOG_INFO("Game: initializing core systems...");

    if (!InputManager::getInstance().begin()) {
        LOG_ERROR("Game: InputManager init failed");
        return false;
    }
    LOG_INFO("  [OK] InputManager");

    if (!DisplayManager::getInstance().begin()) {
        LOG_ERROR("Game: DisplayManager init failed");
        return false;
    }
    LOG_INFO("  [OK] DisplayManager");

    if (!AudioManager::getInstance().begin()) {
        LOG_WARN("Game: AudioManager init failed (audio disabled)");
    }
    LOG_INFO("  [OK] AudioManager");

    if (!SDManager::getInstance().begin()) {
        LOG_WARN("Game: SDManager init failed (content from flash)");
    }
    LOG_INFO("  [OK] SDManager");

    return true;
}

// ============================================================================
// ENGINE INIT
// ============================================================================

bool Game::initEngine() {
    LOG_INFO("Game: initializing engine...");

    // Scene Manager
    m_sceneManager = new SceneManager();
    if (!m_sceneManager) {
        LOG_ERROR("Game: SceneManager allocation failed");
        return false;
    }
    LOG_INFO("  [OK] SceneManager");

    LOG_INFO("  [OK] Engine");
    return true;
}

// ============================================================================
// SYSTEMS INIT
// ============================================================================

bool Game::initSystems() {
    LOG_INFO("Game: initializing game systems...");

    // Route Manager (must be first — other systems depend on route data)
    m_routeManager = &RouteManager::getInstance();
    m_routeManager->begin();
    LOG_INFO("  [OK] RouteManager");

    // Quest Manager
    m_questManager = new QuestManager();
    if (!m_questManager) {
        LOG_ERROR("Game: QuestManager allocation failed");
        return false;
    }
    m_questManager->begin();
    LOG_INFO("  [OK] QuestManager");

    // Inventory
    m_inventory = &Inventory::getInstance();
    m_inventory->begin();
    LOG_INFO("  [OK] Inventory");

    // Save System
    m_saveSystem = &SaveSystem::getInstance();
    m_saveSystem->begin();
    LOG_INFO("  [OK] SaveSystem");

    // Dialog Scene
    m_dialogScene = new DialogScene();
    if (!m_dialogScene) {
        LOG_ERROR("Game: DialogScene allocation failed");
        return false;
    }
    LOG_INFO("  [OK] DialogScene");

    // Battle Scene
    m_battleScene = new BattleScene();
    if (!m_battleScene) {
        LOG_ERROR("Game: BattleScene allocation failed");
        return false;
    }
    LOG_INFO("  [OK] BattleScene");

    // Cutscene Scene
    m_cutsceneScene = new CutsceneScene();
    if (!m_cutsceneScene) {
        LOG_ERROR("Game: CutsceneScene allocation failed");
        return false;
    }
    LOG_INFO("  [OK] CutsceneScene");

    // Overworld Scene
    m_overworld = new OverworldScene();
    if (!m_overworld) {
        LOG_ERROR("Game: OverworldScene allocation failed");
        return false;
    }
    LOG_INFO("  [OK] OverworldScene");

    // Resonance System
    m_resonance = new ResonanceSystem();
    if (!m_resonance) {
        LOG_WARN("Game: ResonanceSystem allocation failed (disabled)");
    } else {
        m_resonance->begin();
    }
    LOG_INFO("  [OK] ResonanceSystem");

    LOG_INFO("  [OK] Game Systems");
    return true;
}

// ============================================================================
// RENDERING INIT
// ============================================================================

bool Game::initRendering() {
    LOG_INFO("Game: initializing rendering...");

    // Sprite Renderer (singleton)
    m_spriteRenderer = &SpriteRenderer::getInstance();
    m_spriteRenderer->begin();
    LOG_INFO("  [OK] SpriteRenderer");

    // Tilemap Renderer (singleton)
    m_tilemapRenderer = &TilemapRenderer::getInstance();
    m_tilemapRenderer->begin();
    LOG_INFO("  [OK] TilemapRenderer");

    // HUD (singleton)
    m_hud = &HUD::getInstance();
    m_hud->begin();
    LOG_INFO("  [OK] HUD");

    // Effects (initialized but not started — activated on demand)
    RainEffect::getInstance().begin();
    GlitchEffect::getInstance().begin();
    CRTEffect::getInstance().begin();
    LOG_INFO("  [OK] Effects");

    // Transitions
    FadeTransition::getInstance().begin();
    LOG_INFO("  [OK] Transitions");

    // Sprite Animation
    SpriteAnimation::getInstance().begin();
    LOG_INFO("  [OK] SpriteAnimation");

    LOG_INFO("  [OK] Rendering");
    return true;
}

// ============================================================================
// CONTENT INIT
// ============================================================================

bool Game::initContent() {
    LOG_INFO("Game: loading content...");

    // Initialize content manager
    if (!ContentManager::getInstance().begin()) {
        LOG_ERROR("Game: ContentManager init failed");
        return false;
    }
    LOG_INFO("  [OK] ContentManager");

    // Preload Arc 1 content
    ContentManager::getInstance().setCurrentArc(1);
    LOG_INFO("  [OK] Arc 1 content loaded");

    LOG_INFO("  [OK] Content");
    return true;
}

// ============================================================================
// MAIN GAME LOOP
// ============================================================================

void Game::gameLoop() {
    while (m_running) {
        syncFPS();
        InputManager::getInstance().update();
        update();
        render();
        AudioManager::getInstance().update();

        m_frameCount++;
        if (millis() - m_fpsTimer >= 1000) {
            m_fps = m_frameCount;
            m_frameCount = 0;
            m_fpsTimer = millis();

#if DEBUG_SHOW_FPS
            LOG_VERBOSE_F("FPS: %.1f, Heap: %d, PSRAM: %d",
                         m_fps, getFreeHeap(), getFreePSRAM());
#endif
        }
    }
}

// ============================================================================
// UPDATE
// ============================================================================

void Game::update() {
    switch (m_state) {
        case GameState::TITLE:
            if (InputManager::getInstance().justPressed(Button::A)) {
                setState(GameState::OVERWORLD);
            }
            break;

        case GameState::OVERWORLD:
            if (m_overworld) {
                m_overworld->update();
            }
            break;

        case GameState::DIALOG:
            if (m_dialogScene) {
                m_dialogScene->update();
            }
            break;

        case GameState::BATTLE:
            if (m_battleScene) {
                m_battleScene->update();
            }
            break;

        case GameState::CUTSCENE:
            if (m_cutsceneScene) {
                m_cutsceneScene->update();
            }
            break;

        case GameState::MENU:
            // TODO: menu update
            break;

        case GameState::RESONANCE:
            if (m_resonance) {
                m_resonance->update();
            }
            break;

        default:
            break;
    }
}

// ============================================================================
// RENDER
// ============================================================================

void Game::render() {
    DisplayManager& display = DisplayManager::getInstance();
    display.beginFrame();

    switch (m_state) {
        case GameState::BOOT:
        case GameState::TITLE:
            display.clearBuffer(Palette::DEEP_BG);
            // TODO: render title screen with MenuUI
            break;

        case GameState::OVERWORLD:
            if (m_overworld) {
                m_overworld->render();
            }
            if (m_hud) {
                m_hud->render();
            }
            break;

        case GameState::DIALOG:
            if (m_overworld) {
                m_overworld->render();
            }
            if (m_dialogScene) {
                m_dialogScene->render();
            }
            break;

        case GameState::BATTLE:
            if (m_battleScene) {
                m_battleScene->render();
            }
            break;

        case GameState::CUTSCENE:
            if (m_cutsceneScene) {
                m_cutsceneScene->render();
            }
            break;

        case GameState::MENU:
            if (m_overworld) {
                m_overworld->render();
            }
            // TODO: render menu overlay
            break;

        default:
            display.clearBuffer(Palette::DEEP_BG);
            break;
    }

    display.endFrame();
}

// ============================================================================
// STATE MANAGEMENT
// ============================================================================

void Game::setState(GameState newState) {
    m_previousState = m_state;
    m_state = newState;

    LOG_INFO_F("Game: state change: %d -> %d",
               static_cast<int>(m_previousState),
               static_cast<int>(newState));
}

GameState Game::getState() const {
    return m_state;
}

GameState Game::getPreviousState() const {
    return m_previousState;
}

// ============================================================================
// ROUTE
// ============================================================================

void Game::setRoute(GameRoute route) {
    m_route = route;
    LOG_INFO_F("Game: route set to %d", static_cast<int>(route));
}

GameRoute Game::getRoute() const {
    return m_route;
}

// ============================================================================
// ARC
// ============================================================================

uint8_t Game::getCurrentArc() const {
    return m_currentArc;
}

void Game::setCurrentArc(uint8_t arc) {
    if (arc >= 1 && arc <= TOTAL_ARCS) {
        m_currentArc = arc;
        ContentManager::getInstance().setCurrentArc(arc);
        LOG_INFO_F("Game: arc set to %d", arc);
    }
}

// ============================================================================
// SHUTDOWN / RESTART
// ============================================================================

void Game::shutdown() {
    LOG_INFO("Game: shutting down...");
    m_running = false;

    AudioManager::getInstance().stopAll();
    AudioManager::getInstance().unloadAll();

    DisplayManager::getInstance().clear(Palette::BLACK);

    // Clean up dynamically allocated systems
    delete m_sceneManager;    m_sceneManager = nullptr;
    delete m_dialogScene;     m_dialogScene = nullptr;
    delete m_battleScene;     m_battleScene = nullptr;
    delete m_cutsceneScene;   m_cutsceneScene = nullptr;
    delete m_overworld;       m_overworld = nullptr;
    delete m_resonance;       m_resonance = nullptr;
    // Singletons — do not delete
    m_spriteRenderer = nullptr;
    m_tilemapRenderer = nullptr;
    m_hud = nullptr;

    LOG_INFO("Game: shutdown complete");
}

void Game::restart() {
    LOG_INFO("Game: restarting...");
    shutdown();
    delay(500);
    run();
}

// ============================================================================
// MEMORY
// ============================================================================

size_t Game::getFreeHeap() const {
    return heap_caps_get_free_size(MALLOC_CAP_INTERNAL);
}

size_t Game::getFreePSRAM() const {
#if USE_PSRAM_SPRITES
    if (psramFound()) {
        return heap_caps_get_free_size(MALLOC_CAP_SPIRAM);
    }
#endif
    return 0;
}

// ============================================================================
// FPS SYNC
// ============================================================================

void Game::syncFPS() {
    uint32_t currentTime = millis();
    uint32_t elapsed = currentTime - m_lastFrameTime;

    if (elapsed < FRAME_TIME_MS) {
        delay(FRAME_TIME_MS - elapsed);
    }

    m_lastFrameTime = millis();
}

// ============================================================================
// BOOT SCREEN
// ============================================================================

void Game::showBootScreen() {
    DisplayManager& display = DisplayManager::getInstance();
    LOG_INFO("Game: boot sequence started");
}

// ============================================================================
// ERROR SCREEN
// ============================================================================

void Game::showErrorScreen(const char* errorMsg) {
    DisplayManager& display = DisplayManager::getInstance();
    display.clear(Palette::BLACK);
    display.fillRect(0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, Palette::DEEP_BG);
    LOG_ERROR_F("FATAL: %s", errorMsg);
}

// ============================================================================
// ERROR HANDLER
// ============================================================================

void Game::handleError(const char* errorMsg) {
    m_error = true;
    m_running = false;
    showErrorScreen(errorMsg);

    while (true) {
        delay(1000);
    }
}