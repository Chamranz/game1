/*
 * NEON REQUIEM - Arc 1: "The Signal"
 * Quest data: objectives, triggers, and rewards
 */

#ifndef ARC1_QUEST_H
#define ARC1_QUEST_H

#include <Arduino.h>
#include "config/config.h"

// Arc 1 quests
#define ARC1_QUEST_COUNT 3

// Quest definitions: {id, title, description, arcId, routeRequirement}
static const char* ARC1_QUEST_DEFS[][5] = {
    {"arc1_follow_signal", "Follow the Signal", "Investigate the mysterious signal from Sector 7G", "1", "0"},
    {"arc1_find_zero", "Find ZERO", "Locate the mysterious hacker ZERO in the Neon Streets", "1", "0"},
    {"arc1_defeat_hound", "Defeat the City Hound", "Destroy the City Hound security unit in Sector 7G", "1", "0"}
};

// Quest objectives: {questId, objectiveIndex, description, type, targetId, count}
static const uint16_t ARC1_QUEST_OBJECTIVES[][6] = {
    {0, 0, 0, 0, 0, 1},  // Follow signal: reach Sector 7G entrance
    {0, 1, 0, 1, 1, 1},  // Follow signal: investigate the terminal
    {1, 0, 0, 2, 0, 1},  // Find ZERO: talk to ZERO
    {2, 0, 0, 3, 0, 1},  // Defeat Hound: defeat City Hound boss
    {2, 1, 0, 4, 0, 1}   // Defeat Hound: escape Sector 7G
};

// Quest rewards: {questId, expReward, creditsReward, itemId}
static const uint16_t ARC1_QUEST_REWARDS[][4] = {
    {0, 50, 100, 0},      // Signal: 50XP, 100cr
    {1, 30, 50, 1},       // Find ZERO: 30XP, 50cr, item "Data Shard"
    {2, 100, 200, 2}      // Defeat Hound: 100XP, 200cr, item "Hound Core"
};

// Quest triggers: {triggerType, triggerId, questId, action}
// triggerType: 0=enter_map, 1=talk_npc, 2=defeat_enemy, 3=collect_item
static const uint16_t ARC1_QUEST_TRIGGERS[][4] = {
    {0, 2, 0, 0},   // Enter Sector 7G -> start "Follow the Signal"
    {1, 0, 1, 0},   // Talk to ZERO -> start "Find ZERO"
    {2, 0, 2, 0}    // Defeat City Hound -> start "Defeat the Hound"
};

#endif // ARC1_QUEST_H