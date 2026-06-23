// ============================================================================
// NEON REQUIEM — Overworld Scene (Header)
// ============================================================================
// Сцена свободного перемещения по карте. Управляет камерой, NPC,
// переходами между локациями и взаимодействиями.
// ============================================================================

#ifndef NEON_REQUIEM_OVERWORLD_SCENE_H
#define NEON_REQUIEM_OVERWORLD_SCENE_H

#include <cstdint>
#include <Arduino.h>
#include "config/config.h"
#include "../../engine/scene_manager.h"
#include "../../engine/player.h"

class OverworldScene : public Scene {
public:
    OverworldScene();
    virtual ~OverworldScene() = default;

    /// Загрузить карту
    bool loadMap(const char* mapName);

    virtual void update() override;
    virtual void render() override;
    virtual void onEnter() override;
    virtual void onExit() override;

    /// Получить игрока
    Player& getPlayer() { return m_player; }

    /// Получить позицию камеры
    int32_t getCameraX() const { return m_cameraX; }
    int32_t getCameraY() const { return m_cameraY; }

private:
    /// Обновление камеры (следит за игроком)
    void updateCamera();

    /// Проверка переходов между локациями
    void checkTransitions();

    /// Проверка взаимодействия с NPC
    void checkInteraction();

    Player m_player;
    int32_t m_cameraX, m_cameraY;
    char m_currentMap[32];
    bool m_mapLoaded;
};

#endif