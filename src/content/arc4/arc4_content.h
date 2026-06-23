/*
 * NEON REQUIEM - Arc 4: "The Admin Spire"
 * Battle, cutscene, map, and quest data
 * KAI ascends the Spire, confronts the Admin Avatar
 */

#ifndef ARC4_CONTENT_H
#define ARC4_CONTENT_H
#include <Arduino.h>

// Battle — Spire guardians + Admin Avatar boss (4 phases)
static const char* ARC4_ENEMIES[] = {"Admin Avatar","Elite Sentinel","Data Golem","Void Specter"};
static const uint8_t ARC4_ENEMY_STATS[][6] = {{150,18,10,6,100,150},{35,8,5,5,20,30},{40,10,6,4,25,35},{30,7,4,6,18,22}};
// Boss phases: {phaseNumber, hpThreshold, attackPattern, bulletCount, speed, color}
// color is uint16_t (RGB565), so array must be uint16_t
static const uint16_t ARC4_BOSS_PHASES[][6] = {{0,150,0,10,4,0xFFFF},{1,120,1,12,5,0xF800},{2,80,2,14,7,0xF81F},{3,40,3,16,8,0x001F}};
#define ARC4_BOSS_PHASE_COUNT 4

// Cutscene — Spire ascent revelations
static const uint16_t ARC4_CUTSCENE_ASCENT[][5] = {{60,0x0000,0,0},{90,0x0010,1,30},{120,0xFFFF,2,60},{90,0xF800,3,80},{60,0x0000,0,0},{180,0x0010,1,100}};
static const uint16_t ARC4_CUTSCENE_AVATAR[][5] = {{30,0x0000,0,0},{60,0xFFFF,2,90},{90,0xF81F,3,120},{120,0x0000,0,0},{60,0x0010,1,50}};

// Map — Spire interior (ascending floors)
static const uint16_t ARC4_MAP_DEFS[][6] = {{0,30,25,0x0000,15,12},{1,35,30,0x0010,17,15},{2,40,35,0xFFFF,20,17},{3,50,40,0xF800,25,20}};
static const int8_t ARC4_MAP_CONNECTIONS[][5] = {{0,-1,1,-1,-1},{1,0,2,-1,-1},{2,1,3,-1,-1},{3,2,-1,-1,-1}};
static const void* ARC4_NPC_PLACEMENT[][5] = {{(const void*)0,(const void*)0,(const void*)10,(const void*)12,"arc4_terminal"},{(const void*)1,(const void*)1,(const void*)20,(const void*)18,"arc4_ghost"},{(const void*)2,(const void*)2,(const void*)30,(const void*)22,"arc4_echo"},{(const void*)3,(const void*)3,(const void*)35,(const void*)30,"arc4_avatar_terminal"}};

// Quest
#define ARC4_QUEST_COUNT 3
static const char* ARC4_QUEST_DEFS[][5] = {{"arc4_enter_spire","Enter the Admin Spire","Breach the Admin Spire and begin the ascent","4","0"},{"arc4_discover_truth","Discover the Architect's Truth","Uncover the Architect's logs and learn the city's origin","4","0"},{"arc4_defeat_avatar","Defeat the Admin Avatar","Destroy the Architect's digital projection","4","0"}};
static const uint16_t ARC4_QUEST_OBJECTIVES[][6] = {{0,0,0,0,2,1},{0,1,0,1,1,1},{1,0,0,2,0,1},{2,0,0,3,0,1}};
static const uint16_t ARC4_QUEST_REWARDS[][4] = {{0,100,300,8},{1,150,500,9},{2,200,750,10}};
#endif