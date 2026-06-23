// ============================================================================
// NEON REQUIEM — Overworld Scene (Implementation)
// ============================================================================

#include "overworld_scene.h"
#include "../../core/input/input_manager.h"
#include "../../core/display/display_manager.h"
#include "../../core/storage/sd_manager.h"

OverworldScene::OverworldScene()
    : Scene(SceneType::OVERWORLD)
    , m_cameraX(0), m_cameraY(0), m_mapLoaded(false)
{
    m_currentMap[0] = '\0';
}

bool OverworldScene::loadMap(const char* mapName) {
    strncpy(m_currentMap, mapName, sizeof(m_currentMap) - 1);

    char path[64];
    snprintf(path, sizeof(path), "%s%s.json", SD_PATH_MAPS, mapName);

    if (!SDManager::getInstance().fileExists(path)) {
        LOG_ERROR_F("Overworld: map not found: %s", path);
        return false;
    }

    m_player.begin();
    m_mapLoaded = true;
    LOG_INFO_F("Overworld: loaded map: %s", mapName);
    return true;
}

void OverworldScene::update() {
    if (!m_mapLoaded) return;

    m_player.update();
    updateCamera();
    checkTransitions();
    checkInteraction();
}

void OverworldScene::render() {
    if (!m_mapLoaded) return;

    DisplayManager& display = DisplayManager::getInstance();
    display.clearBuffer(Palette::DEEP_BG);

    // TODO: отрисовка тайловой карты через TilemapRenderer
    // Пока — placeholder сетка
    for (int y = 0; y < DISPLAY_HEIGHT; y += TILE_SIZE) {
        for (int x = 0; x < DISPLAY_WIDTH; x += TILE_SIZE) {
            display.drawRect(x, y, TILE_SIZE, TILE_SIZE, Palette::CONCRETE);
        }
    }

    // Отрисовка игрока
    m_player.render(m_cameraX, m_cameraY);
}

void OverworldScene::updateCamera() {
    // Камера следует за игроком с центрированием
    m_cameraX = m_player.getX() - DISPLAY_WIDTH / 2;
    m_cameraY = m_player.getY() - DISPLAY_HEIGHT / 2;

    // Ограничение границами карты
    m_cameraX = CLAMP(m_cameraX, 0, MAP_WIDTH_PX - DISPLAY_WIDTH);
    m_cameraY = CLAMP(m_cameraY, 0, MAP_HEIGHT_PX - DISPLAY_HEIGHT);
}

void OverworldScene::checkTransitions() {
    // TODO: проверка триггеров перехода между локациями
}

void OverworldScene::checkInteraction() {
    InputManager& input = InputManager::getInstance();

    if (input.justPressed(Button::A)) {
        // TODO: проверка NPC в радиусе взаимодействия
        LOG_VERBOSE("Overworld: interaction check");
    }
}

void OverworldScene::onEnter() { Scene::onEnter(); LOG_INFO("Overworld: entered"); }
void OverworldScene::onExit() { Scene::onExit(); LOG_INFO("Overworld: exited"); }