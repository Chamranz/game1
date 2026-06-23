// ============================================================================
// NEON REQUIEM — NPC (Header)
// ============================================================================

#ifndef NEON_REQUIEM_NPC_H
#define NEON_REQUIEM_NPC_H

#include <cstdint>
#include <Arduino.h>
#include "config/config.h"

struct NPCData {
    char     id[24];          /// Уникальный ID NPC
    char     name[24];        /// Имя для диалогов
    int16_t  tileX, tileY;    /// Позиция на карте
    uint16_t color;           /// Цвет спрайта
    uint16_t dialogId;        /// ID диалога
    bool     isMerchant;      /// Торговец?
    bool     isQuestGiver;    /// Выдаёт квест?
    uint16_t questId;         /// ID квеста
    uint8_t  movePattern;     /// 0=стоит, 1=ходит, 2=патруль
};

class NPC {
public:
    NPC();
    void begin(const NPCData& data);
    void update();
    void render(int32_t cameraX, int32_t cameraY);
    bool isInRange(float playerX, float playerY, int range) const;
    const NPCData& getData() const { return m_data; }

private:
    NPCData m_data;
    int m_moveTimer;
    int m_moveDir;
};

#endif