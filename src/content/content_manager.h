// ============================================================================
// NEON REQUIEM — Content Manager
// ============================================================================
// Central hub that ties all arc content together.
// Provides access to dialog, battle, cutscene, map, and quest data
// for all 5 story arcs.
// ============================================================================

#ifndef CONTENT_MANAGER_H
#define CONTENT_MANAGER_H

#include <Arduino.h>
#include "config/config.h"

// ============================================================================
// ARC CONTENT INCLUDES
// ============================================================================

#include "arc1/arc1_dialog.h"
#include "arc1/arc1_battle.h"
#include "arc1/arc1_cutscene.h"
#include "arc1/arc1_map.h"
#include "arc1/arc1_quest.h"

#include "arc2/arc2_dialog.h"
#include "arc2/arc2_content.h"

#include "arc3/arc3_dialog.h"
#include "arc3/arc3_content.h"

#include "arc4/arc4_dialog.h"
#include "arc4/arc4_content.h"

#include "arc5/arc5_dialog.h"
#include "arc5/arc5_content.h"

// ============================================================================
// CONTENT MANAGER CLASS
// ============================================================================

class ContentManager {
public:
    /// Получить единственный экземпляр
    static ContentManager& getInstance() {
        static ContentManager instance;
        return instance;
    }

    /// Инициализация
    bool begin() {
        LOG_INFO("ContentManager: initializing...");
        m_currentArc = 1;
        LOG_INFO("ContentManager: ready");
        return true;
    }

    /// Установить текущую арку
    void setCurrentArc(uint8_t arc) {
        if (arc >= 1 && arc <= TOTAL_ARCS) {
            m_currentArc = arc;
        }
    }

    /// Получить текущую арку
    uint8_t getCurrentArc() const { return m_currentArc; }

    // ========================================================================
    // DIALOG ACCESSORS
    // ========================================================================

    /// Получить массив диалога по ID
    const char** getDialog(const char* dialogId) {
        // Arc 1 dialogs
        if (strcmp(dialogId, "arc1_awakening") == 0) return ARC1_DIALOG_INTRO;
        if (strcmp(dialogId, "arc1_meet_zero") == 0) return ARC1_DIALOG_ZERO_MEET;
        if (strcmp(dialogId, "arc1_choice") == 0) return ARC1_DIALOG_ZERO_MEET;
        if (strcmp(dialogId, "arc1_follow") == 0) return ARC1_DIALOG_FOLLOW_ZERO;
        if (strcmp(dialogId, "arc1_investigate") == 0) return ARC1_DIALOG_ALONE;
        if (strcmp(dialogId, "arc1_report") == 0) return ARC1_DIALOG_REPORT;
        if (strcmp(dialogId, "arc1_boss_intro") == 0) return ARC1_DIALOG_BOSS_INTRO;
        if (strcmp(dialogId, "arc1_boss_defeat") == 0) return ARC1_DIALOG_BOSS_DEFEAT;

        // Arc 2 dialogs
        if (strcmp(dialogId, "arc2_oracle") == 0) return ARC2_DIALOG_ORACLE;
        if (strcmp(dialogId, "arc2_help") == 0) return ARC2_DIALOG_HELP;
        if (strcmp(dialogId, "arc2_control") == 0) return ARC2_DIALOG_CONTROL;
        if (strcmp(dialogId, "arc2_truth") == 0) return ARC2_DIALOG_TRUTH;
        if (strcmp(dialogId, "arc2_boss_intro") == 0) return ARC2_BOSS_INTRO;
        if (strcmp(dialogId, "arc2_boss_defeat") == 0) return ARC2_BOSS_DEFEAT;

        // Arc 3 dialogs
        if (strcmp(dialogId, "arc3_resistance") == 0) return ARC3_DIALOG_RESISTANCE;
        if (strcmp(dialogId, "arc3_betrayal") == 0) return ARC3_DIALOG_BETRAYAL;
        if (strcmp(dialogId, "arc3_boss_intro") == 0) return ARC3_BOSS_INTRO;
        if (strcmp(dialogId, "arc3_boss_defeat") == 0) return ARC3_BOSS_DEFEAT;

        // Arc 4 dialogs
        if (strcmp(dialogId, "arc4_entry") == 0) return ARC4_DIALOG_ENTRY;
        if (strcmp(dialogId, "arc4_ascent") == 0) return ARC4_DIALOG_ASCENT;
        if (strcmp(dialogId, "arc4_logs") == 0) return ARC4_DIALOG_LOGS;
        if (strcmp(dialogId, "arc4_resonance") == 0) return ARC4_DIALOG_RESONANCE;
        if (strcmp(dialogId, "arc4_erase") == 0) return ARC4_DIALOG_ERASE;
        if (strcmp(dialogId, "arc4_glitch") == 0) return ARC4_DIALOG_GLITCH;
        if (strcmp(dialogId, "arc4_boss_intro") == 0) return ARC4_BOSS_INTRO;
        if (strcmp(dialogId, "arc4_boss_phase2") == 0) return ARC4_BOSS_PHASE2;
        if (strcmp(dialogId, "arc4_boss_phase3") == 0) return ARC4_BOSS_PHASE3;
        if (strcmp(dialogId, "arc4_boss_defeat") == 0) return ARC4_BOSS_DEFEAT;

        // Arc 5 dialogs
        if (strcmp(dialogId, "arc5_throne") == 0) return ARC5_DIALOG_THRONE;
        if (strcmp(dialogId, "arc5_resonance") == 0) return ARC5_DIALOG_RESONANCE;
        if (strcmp(dialogId, "arc5_erase") == 0) return ARC5_DIALOG_ERASE;
        if (strcmp(dialogId, "arc5_glitch") == 0) return ARC5_DIALOG_GLITCH;
        if (strcmp(dialogId, "arc5_boss_intro") == 0) return ARC5_BOSS_INTRO;
        if (strcmp(dialogId, "arc5_boss_phase2") == 0) return ARC5_BOSS_PHASE2;
        if (strcmp(dialogId, "arc5_boss_phase3") == 0) return ARC5_BOSS_PHASE3;
        if (strcmp(dialogId, "arc5_boss_defeat") == 0) return ARC5_BOSS_DEFEAT;

        LOG_WARN_F("ContentManager: unknown dialog ID: %s", dialogId);
        return nullptr;
    }

    // ========================================================================
    // BATTLE DATA ACCESSORS
    // ========================================================================

    /// Получить список врагов для арки
    const char** getEnemies(uint8_t arc) {
        switch (arc) {
            case 1: return ARC1_ENEMIES;
            case 2: return ARC2_ENEMIES;
            case 3: return ARC3_ENEMIES;
            case 4: return ARC4_ENEMIES;
            case 5: return ARC5_ENEMIES;
            default: return nullptr;
        }
    }

    /// Получить статистики врагов для арки
    const uint8_t* getEnemyStats(uint8_t arc) {
        switch (arc) {
            case 1: return (const uint8_t*)ARC1_ENEMY_STATS;
            case 2: return (const uint8_t*)ARC2_ENEMY_STATS;
            case 3: return (const uint8_t*)ARC3_ENEMY_STATS;
            case 4: return (const uint8_t*)ARC4_ENEMY_STATS;
            case 5: return (const uint8_t*)ARC5_ENEMY_STATS;
            default: return nullptr;
        }
    }

    /// Получить фазы босса для арки
    const uint8_t* getBossPhases(uint8_t arc) {
        switch (arc) {
            case 1: return (const uint8_t*)ARC1_BOSS_PHASES;
            case 2: return (const uint8_t*)ARC2_BOSS_PHASES;
            case 3: return (const uint8_t*)ARC3_BOSS_PHASES;
            case 4: return (const uint8_t*)ARC4_BOSS_PHASES;
            case 5: return (const uint8_t*)ARC5_BOSS_PHASES;
            default: return nullptr;
        }
    }

    /// Получить количество врагов для арки
    uint8_t getEnemyCount(uint8_t arc) {
        switch (arc) {
            case 1: return ARC1_ENEMY_COUNT;
            case 2: return 4;
            case 3: return 4;
            case 4: return 4;
            case 5: return 4;
            default: return 0;
        }
    }

    /// Получить количество фаз босса для арки
    uint8_t getBossPhaseCount(uint8_t arc) {
        switch (arc) {
            case 1: return ARC1_BOSS_PHASE_COUNT;
            case 2: return 3;
            case 3: return 4;
            case 4: return 4;
            case 5: return 5;
            default: return 0;
        }
    }

    // ========================================================================
    // CUTSCENE DATA ACCESSORS
    // ========================================================================

    /// Получить катсцену по ID
    const uint16_t* getCutscene(const char* cutsceneId, uint16_t& frameCount) {
        // Arc 1
        if (strcmp(cutsceneId, "arc1_signal") == 0) {
            frameCount = sizeof(ARC1_CUTSCENE_SIGNAL) / sizeof(ARC1_CUTSCENE_SIGNAL[0]);
            return (const uint16_t*)ARC1_CUTSCENE_SIGNAL;
        }
        if (strcmp(cutsceneId, "arc1_first_battle") == 0) {
            frameCount = sizeof(ARC1_CUTSCENE_BATTLE) / sizeof(ARC1_CUTSCENE_BATTLE[0]);
            return (const uint16_t*)ARC1_CUTSCENE_BATTLE;
        }
        if (strcmp(cutsceneId, "arc1_complete") == 0) {
            frameCount = sizeof(ARC1_CUTSCENE_COMPLETE) / sizeof(ARC1_CUTSCENE_COMPLETE[0]);
            return (const uint16_t*)ARC1_CUTSCENE_COMPLETE;
        }

        // Arc 2
        if (strcmp(cutsceneId, "arc2_oracle") == 0) {
            frameCount = 6;
            return (const uint16_t*)ARC2_CUTSCENE_ORACLE;
        }
        if (strcmp(cutsceneId, "arc2_core") == 0) {
            frameCount = 4;
            return (const uint16_t*)ARC2_CUTSCENE_CORE;
        }

        // Arc 3
        if (strcmp(cutsceneId, "arc3_uprising") == 0) {
            frameCount = 5;
            return (const uint16_t*)ARC3_CUTSCENE_UPRISING;
        }
        if (strcmp(cutsceneId, "arc3_betrayal") == 0) {
            frameCount = 3;
            return (const uint16_t*)ARC3_CUTSCENE_BETRAYAL;
        }

        // Arc 4
        if (strcmp(cutsceneId, "arc4_ascent") == 0) {
            frameCount = 6;
            return (const uint16_t*)ARC4_CUTSCENE_ASCENT;
        }
        if (strcmp(cutsceneId, "arc4_avatar") == 0) {
            frameCount = 5;
            return (const uint16_t*)ARC4_CUTSCENE_AVATAR;
        }

        // Arc 5
        if (strcmp(cutsceneId, "arc5_final") == 0) {
            frameCount = 6;
            return (const uint16_t*)ARC5_CUTSCENE_FINAL;
        }
        if (strcmp(cutsceneId, "arc5_resonance_end") == 0) {
            frameCount = 4;
            return (const uint16_t*)ARC5_CUTSCENE_RESONANCE_END;
        }
        if (strcmp(cutsceneId, "arc5_erase_end") == 0) {
            frameCount = 4;
            return (const uint16_t*)ARC5_CUTSCENE_ERASE_END;
        }
        if (strcmp(cutsceneId, "arc5_glitch_end") == 0) {
            frameCount = 4;
            return (const uint16_t*)ARC5_CUTSCENE_GLITCH_END;
        }

        LOG_WARN_F("ContentManager: unknown cutscene ID: %s", cutsceneId);
        frameCount = 0;
        return nullptr;
    }

    // ========================================================================
    // MAP DATA ACCESSORS
    // ========================================================================

    /// Получить определения карт для арки
    const uint16_t* getMapDefs(uint8_t arc) {
        switch (arc) {
            case 1: return (const uint16_t*)ARC1_MAP_DEFS;
            case 2: return (const uint16_t*)ARC2_MAP_DEFS;
            case 3: return (const uint16_t*)ARC3_MAP_DEFS;
            case 4: return (const uint16_t*)ARC4_MAP_DEFS;
            case 5: return (const uint16_t*)ARC5_MAP_DEFS;
            default: return nullptr;
        }
    }

    /// Получить соединения карт для арки
    const int8_t* getMapConnections(uint8_t arc) {
        switch (arc) {
            case 1: return (const int8_t*)ARC1_MAP_CONNECTIONS;
            case 2: return (const int8_t*)ARC2_MAP_CONNECTIONS;
            case 3: return (const int8_t*)ARC3_MAP_CONNECTIONS;
            case 4: return (const int8_t*)ARC4_MAP_CONNECTIONS;
            case 5: return (const int8_t*)ARC5_MAP_CONNECTIONS;
            default: return nullptr;
        }
    }

    /// Получить размещение NPC для арки
    const uint16_t* getNPCPlacement(uint8_t arc) {
        switch (arc) {
            case 1: return (const uint16_t*)ARC1_NPC_PLACEMENT;
            case 2: return (const uint16_t*)ARC2_NPC_PLACEMENT;
            case 3: return (const uint16_t*)ARC3_NPC_PLACEMENT;
            case 4: return (const uint16_t*)ARC4_NPC_PLACEMENT;
            case 5: return (const uint16_t*)ARC5_NPC_PLACEMENT;
            default: return nullptr;
        }
    }

    /// Получить количество карт для арки
    uint8_t getMapCount(uint8_t arc) {
        switch (arc) {
            case 1: return ARC1_MAP_COUNT;
            case 2: return 3;
            case 3: return 3;
            case 4: return 4;
            case 5: return 4;
            default: return 0;
        }
    }

    // ========================================================================
    // QUEST DATA ACCESSORS
    // ========================================================================

    /// Получить количество квестов для арки
    uint8_t getQuestCount(uint8_t arc) {
        switch (arc) {
            case 1: return ARC1_QUEST_COUNT;
            case 2: return ARC2_QUEST_COUNT;
            case 3: return ARC3_QUEST_COUNT;
            case 4: return ARC4_QUEST_COUNT;
            case 5: return ARC5_QUEST_COUNT;
            default: return 0;
        }
    }

    /// Получить определения квестов для арки
    const char* const* getQuestDefs(uint8_t arc) {
        switch (arc) {
            case 1: return (const char* const*)ARC1_QUEST_DEFS;
            case 2: return (const char* const*)ARC2_QUEST_DEFS;
            case 3: return (const char* const*)ARC3_QUEST_DEFS;
            case 4: return (const char* const*)ARC4_QUEST_DEFS;
            case 5: return (const char* const*)ARC5_QUEST_DEFS;
            default: return nullptr;
        }
    }

    /// Получить цели квестов для арки
    const uint16_t* getQuestObjectives(uint8_t arc) {
        switch (arc) {
            case 1: return (const uint16_t*)ARC1_QUEST_OBJECTIVES;
            case 2: return (const uint16_t*)ARC2_QUEST_OBJECTIVES;
            case 3: return (const uint16_t*)ARC3_QUEST_OBJECTIVES;
            case 4: return (const uint16_t*)ARC4_QUEST_OBJECTIVES;
            case 5: return (const uint16_t*)ARC5_QUEST_OBJECTIVES;
            default: return nullptr;
        }
    }

    /// Получить награды квестов для арки
    const uint16_t* getQuestRewards(uint8_t arc) {
        switch (arc) {
            case 1: return (const uint16_t*)ARC1_QUEST_REWARDS;
            case 2: return (const uint16_t*)ARC2_QUEST_REWARDS;
            case 3: return (const uint16_t*)ARC3_QUEST_REWARDS;
            case 4: return (const uint16_t*)ARC4_QUEST_REWARDS;
            case 5: return (const uint16_t*)ARC5_QUEST_REWARDS;
            default: return nullptr;
        }
    }

    // ========================================================================
    // ENCOUNTER DATA ACCESSORS
    // ========================================================================

    /// Получить конфигурации встреч для арки
    const uint8_t* getEncounters(uint8_t arc) {
        switch (arc) {
            case 1: return (const uint8_t*)ARC1_ENCOUNTERS;
            default: return nullptr;
        }
    }

    /// Получить количество встреч для арки
    uint8_t getEncounterCount(uint8_t arc) {
        switch (arc) {
            case 1: return sizeof(ARC1_ENCOUNTERS) / sizeof(ARC1_ENCOUNTERS[0]);
            default: return 0;
        }
    }

private:
    ContentManager() : m_currentArc(1) {}
    ContentManager(const ContentManager&) = delete;
    ContentManager& operator=(const ContentManager&) = delete;

    uint8_t m_currentArc;
};

#endif // CONTENT_MANAGER_H