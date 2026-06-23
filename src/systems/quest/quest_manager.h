// ============================================================================
// NEON REQUIEM — Quest Manager (Header)
// ============================================================================

#ifndef NEON_REQUIEM_QUEST_MANAGER_H
#define NEON_REQUIEM_QUEST_MANAGER_H

#include <cstdint>
#include <Arduino.h>
#include "config/config.h"

enum class QuestStatus : uint8_t {
    INACTIVE = 0,
    ACTIVE   = 1,
    COMPLETE = 2,
    FAILED   = 3
};

struct QuestData {
    uint16_t    id;
    char        title[48];
    char        description[128];
    QuestStatus status;
    uint8_t     arcId;
    int8_t      routeRequirement;
    uint8_t     objectives[4];
    uint8_t     objectiveCount;
    uint8_t     objectiveProgress[4];
};

class QuestManager {
public:
    QuestManager();
    void begin();
    void update();

    bool addQuest(uint16_t questId);
    bool completeQuest(uint16_t questId);
    bool failQuest(uint16_t questId);
    bool advanceObjective(uint16_t questId, uint8_t objectiveIndex);

    QuestData* getQuest(uint16_t questId);
    int getActiveQuestCount() const;
    QuestData* getActiveQuests();

    bool isQuestComplete(uint16_t questId) const;
    bool isQuestActive(uint16_t questId) const;

private:
    QuestData m_quests[MAX_ACTIVE_QUESTS];
    int m_questCount;
};

#endif