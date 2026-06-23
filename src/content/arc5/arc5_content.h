/*
 * NEON REQUIEM - Arc 5: "The Final Truth"
 * Battle, cutscene, map, and quest data
 * KAI confronts the Architect, three distinct endings
 */

#ifndef ARC5_CONTENT_H
#define ARC5_CONTENT_H
#include <Arduino.h>

// Battle — Architect's final guardians + The Architect boss (5 phases)
static const char* ARC5_ENEMIES[] = {"The Architect","Void Walker","Omega Sentinel","Reality Anchor"};
static const uint8_t ARC5_ENEMY_STATS[][6] = {{200,20,12,8,120,200},{45,10,6,6,25,35},{50,12,8,5,30,40},{35,8,5,7,22,28}};
// Boss phases: {phaseNumber, hpThreshold, attackPattern, bulletCount, speed, color}
// color is uint16_t (RGB565), so array must be uint16_t
static const uint16_t ARC5_BOSS_PHASES[][6] = {{0,200,0,12,4,0xFFFF},{1,170,1,14,5,0xF800},{2,130,2,16,6,0xF81F},{3,80,3,18,8,0x001F},{4,30,4,20,10,0x07E0}};
#define ARC5_BOSS_PHASE_COUNT 5

// Cutscene — Final revelations and endings
static const uint16_t ARC5_CUTSCENE_FINAL[][5] = {{60,0x0000,0,0},{90,0xFFFF,2,80},{120,0xF800,3,100},{180,0x0010,1,60},{90,0x0000,0,0},{240,0x07E0,2,120}};
static const uint16_t ARC5_CUTSCENE_RESONANCE_END[][5] = {{60,0x0000,0,0},{120,0x07E0,1,40},{180,0xFFFF,2,60},{300,0x0000,0,0}};
static const uint16_t ARC5_CUTSCENE_ERASE_END[][5] = {{60,0x0000,0,0},{120,0xF800,2,80},{180,0x0000,1,50},{300,0xF800,3,100}};
static const uint16_t ARC5_CUTSCENE_GLITCH_END[][5] = {{60,0x0000,0,0},{120,0xF81F,3,120},{180,0x07E0,2,80},{300,0x0000,0,0}};

// Map — The Architect's domain (final area)
static const uint16_t ARC5_MAP_DEFS[][6] = {{0,40,30,0x0000,20,15},{1,50,40,0xFFFF,25,20},{2,60,50,0xF800,30,25},{3,80,60,0x0010,40,30}};
static const int8_t ARC5_MAP_CONNECTIONS[][5] = {{0,-1,1,-1,-1},{1,0,2,-1,-1},{2,1,3,-1,-1},{3,2,-1,-1,-1}};
static const void* ARC5_NPC_PLACEMENT[][5] = {{(const void*)0,(const void*)0,(const void*)15,(const void*)10,"arc5_echo"},{(const void*)1,(const void*)1,(const void*)25,(const void*)20,"arc5_memory"},{(const void*)2,(const void*)2,(const void*)35,(const void*)25,"arc5_vision"},{(const void*)3,(const void*)3,(const void*)50,(const void*)35,"arc5_architect_throne"}};

// Quest
#define ARC5_QUEST_COUNT 3
static const char* ARC5_QUEST_DEFS[][5] = {{"arc5_reach_throne","Reach the Architect's Throne","Ascend to the top of reality and confront the Architect","5","0"},{"arc5_final_battle","Defeat the Architect","Face the Architect in the final battle for the fate of Neon City","5","0"},{"arc5_choose_fate","Choose the City's Fate","Make the final choice that will determine the future of every soul in Neon City","5","0"}};
static const uint16_t ARC5_QUEST_OBJECTIVES[][6] = {{0,0,0,0,3,1},{0,1,0,1,2,1},{1,0,0,2,0,1},{2,0,0,3,0,1}};
static const uint16_t ARC5_QUEST_REWARDS[][4] = {{0,200,500,11},{1,300,1000,12},{2,500,2000,13}};
#endif