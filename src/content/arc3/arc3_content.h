/*
 * NEON REQUIEM - Arc 3: "The Resistance"
 * Battle, cutscene, map, and quest data
 */

#ifndef ARC3_CONTENT_H
#define ARC3_CONTENT_H
#include <Arduino.h>

// Battle
static const char* ARC3_ENEMIES[] = {"Commander Vex","Security Drone","Elite Guard","Turret System"};
static const uint8_t ARC3_ENEMY_STATS[][6] = {{120,15,8,5,80,120},{25,6,3,5,10,15},{30,8,5,4,15,25},{20,10,2,2,12,18}};
// Boss phases: {phaseNumber, hpThreshold, attackPattern, bulletCount, speed, color}
// color is uint16_t (RGB565), so array must be uint16_t
static const uint16_t ARC3_BOSS_PHASES[][6] = {{0,120,0,8,3,0xF800},{1,80,1,10,4,0xFFFF},{2,40,3,12,6,0x001F},{3,15,2,15,7,0xF81F}};
#define ARC3_BOSS_PHASE_COUNT 4

// Cutscene
static const uint16_t ARC3_CUTSCENE_UPRISING[][5] = {{60,0x0000,0,0},{90,0xF800,2,60},{120,0x0000,1,40},{60,0xFFFF,3,80},{180,0x0000,0,0}};
static const uint16_t ARC3_CUTSCENE_BETRAYAL[][5] = {{30,0x0000,0,0},{60,0xF800,2,100},{90,0x0000,0,0}};

// Map
static const uint16_t ARC3_MAP_DEFS[][6] = {{0,50,40,0x0000,25,20},{1,60,45,0xF800,30,22},{2,40,30,0x0000,20,15}};
static const int8_t ARC3_MAP_CONNECTIONS[][5] = {{0,-1,1,-1,-1},{1,0,2,-1,-1},{2,1,-1,-1,-1}};
static const void* ARC3_NPC_PLACEMENT[][5] = {{(const void*)0,(const void*)0,(const void*)20,(const void*)18,"arc3_resistance_leader"},{(const void*)0,(const void*)1,(const void*)30,(const void*)25,"arc3_ally"},{(const void*)1,(const void*)2,(const void*)40,(const void*)30,"arc3_informant"}};

// Quest
#define ARC3_QUEST_COUNT 3
static const char* ARC3_QUEST_DEFS[][5] = {{"arc3_join_resistance","Join the Resistance","Find and join the underground resistance movement","3","0"},{"arc3_infiltrate_spire","Infiltrate the Spire","Gain access to the Admin Spire","3","0"},{"arc3_defeat_vex","Defeat Commander Vex","Eliminate the head of City Security","3","0"}};
static const uint16_t ARC3_QUEST_OBJECTIVES[][6] = {{0,0,0,0,0,1},{0,1,0,1,1,1},{1,0,0,2,0,1},{2,0,0,3,0,1}};
static const uint16_t ARC3_QUEST_REWARDS[][4] = {{0,80,200,5},{1,100,300,6},{2,150,400,7}};
#endif