/*
 * NEON REQUIEM - Arc 1: "The Signal"
 * Battle data: enemies, bosses, and encounter configurations
 */

#ifndef ARC1_BATTLE_H
#define ARC1_BATTLE_H

#include <Arduino.h>
#include "config/config.h"

// Arc 1 enemies
static const char* ARC1_ENEMIES[] = {
    "City Hound",           // Boss
    "Data Wraith",          // Minor enemy
    "Firewall Sprite",      // Minor enemy
    "Corrupted Node"        // Minor enemy
};

// Enemy stats: {hp, attack, defense, speed, expReward, creditsReward}
static const uint8_t ARC1_ENEMY_STATS[][6] = {
    {50, 8, 4, 6, 30, 50},    // City Hound (boss)
    {15, 3, 1, 3, 8, 10},     // Data Wraith
    {12, 4, 2, 5, 6, 8},      // Firewall Sprite
    {20, 2, 3, 2, 10, 15}     // Corrupted Node
};

// Boss phase data: {phaseNumber, hpThreshold, attackPattern, bulletCount, speed, color}
// Note: color is uint16_t (RGB565), so the array must be uint16_t
static const uint16_t ARC1_BOSS_PHASES[][6] = {
    {0, 50, 0, 4, 3, 0x001F},   // Phase 1: Spiral, blue
    {1, 30, 1, 6, 4, 0xF800},   // Phase 2: Wave, red
    {2, 15, 2, 8, 5, 0xFFFF}    // Phase 3: Rain, white (enraged)
};

// Count helpers
#define ARC1_ENEMY_COUNT      4
#define ARC1_BOSS_PHASE_COUNT 3

// Encounter table: {enemyIndex, count, isBoss, dialogId}
static const uint8_t ARC1_ENCOUNTERS[][4] = {
    {1, 2, 0, 0},   // 2x Data Wraith
    {2, 3, 0, 0},   // 3x Firewall Sprite
    {3, 1, 0, 0},   // 1x Corrupted Node
    {0, 1, 1, 1},   // 1x City Hound (boss)
    {1, 1, 0, 0},   // 1x Data Wraith + 1x Firewall Sprite
    {2, 1, 0, 0}
};

// Battle dialog triggers
static const char* ARC1_BATTLE_DIALOG[] = {
    "arc1_hound_intro",   // Boss intro
    "arc1_hound_defeat"   // Boss defeat
};

#endif // ARC1_BATTLE_H