/*
 * NEON REQUIEM - Arc 2: "The Data Heart"
 * Battle, cutscene, map, and quest data
 */

#ifndef ARC2_BATTLE_H
#define ARC2_BATTLE_H
#include <Arduino.h>

static const char* ARC2_ENEMIES[] = {"Core Sentinel","Data Phantom","Virus Construct","Memory Leech"};
static const uint8_t ARC2_ENEMY_STATS[][6] = {{80,12,6,4,50,80},{20,5,2,4,12,15},{18,6,3,6,10,12},{25,4,4,3,15,20}};
// Boss phases: {phaseNumber, hpThreshold, attackPattern, bulletCount, speed, color}
// color is uint16_t (RGB565), so array must be uint16_t
static const uint16_t ARC2_BOSS_PHASES[][6] = {{0,80,0,6,3,0x07E0},{1,50,1,8,4,0xF800},{2,25,3,10,6,0xFFFF}};
#define ARC2_BOSS_PHASE_COUNT 3
#endif

#ifndef ARC2_CUTSCENE_H
#define ARC2_CUTSCENE_H
static const uint16_t ARC2_CUTSCENE_ORACLE[][5] = {{60,0x0000,0,0},{90,0x0010,1,30},{120,0x07E0,2,50},{90,0x0000,0,0},{60,0xFFFF,3,80},{120,0x0000,0,0}};
static const uint16_t ARC2_CUTSCENE_CORE[][5] = {{30,0x0000,0,0},{60,0x07E0,2,60},{90,0x0010,1,40},{120,0x0000,0,0}};
#endif

#ifndef ARC2_MAP_H
#define ARC2_MAP_H
static const uint16_t ARC2_MAP_DEFS[][6] = {{0,45,35,0x07E0,22,17},{1,55,40,0x0000,27,20},{2,50,35,0x0010,25,30}};
static const int8_t ARC2_MAP_CONNECTIONS[][5] = {{0,-1,1,-1,-1},{1,0,2,-1,-1},{2,1,-1,-1,-1}};
static const void* ARC2_NPC_PLACEMENT[][5] = {{(const void*)1,(const void*)0,(const void*)20,(const void*)18,"arc2_oracle"},{(const void*)1,(const void*)1,(const void*)40,(const void*)15,"arc2_resistance"},{(const void*)2,(const void*)2,(const void*)15,(const void*)20,"arc2_terminal"}};
#endif

#ifndef ARC2_QUEST_H
#define ARC2_QUEST_H
#define ARC2_QUEST_COUNT 3
static const char* ARC2_QUEST_DEFS[][5] = {{"arc2_reach_core","Reach the Data Heart","Navigate to the city's central data core","2","0"},{"arc2_meet_oracle","Meet the Oracle","Find the mysterious Oracle in the Data Heart","2","0"},{"arc2_defeat_sentinel","Defeat the Core Sentinel","Destroy the Core Sentinel guarding the archives","2","0"}};
static const uint16_t ARC2_QUEST_OBJECTIVES[][6] = {{0,0,0,0,2,1},{0,1,0,1,1,1},{1,0,0,2,0,1},{2,0,0,3,0,1}};
static const uint16_t ARC2_QUEST_REWARDS[][4] = {{0,60,150,0},{1,40,75,3},{2,120,250,4}};
#endif