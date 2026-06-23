/*
 * NEON REQUIEM - Arc 1: "The Signal"
 * Map data: tile layouts, spawn points, and connections
 */

#ifndef ARC1_MAP_H
#define ARC1_MAP_H

#include <Arduino.h>
#include "config/config.h"

// Arc 1 maps
#define ARC1_MAP_COUNT 3

// Map definitions: {id, width, height, bgColor, spawnX, spawnY}
static const uint16_t ARC1_MAP_DEFS[][6] = {
    {0, 40, 30, 0x001F, 20, 15},   // KAI's Apartment
    {1, 60, 40, 0x0000, 30, 35},   // Neon Streets
    {2, 50, 35, 0x0000, 25, 30}    // Sector 7G
};

// Map connections: {mapIndex, north, south, east, west}
static const int8_t ARC1_MAP_CONNECTIONS[][5] = {
    {0, -1, 1, -1, -1},   // Apartment -> Streets (south)
    {1, 0, 2, -1, -1},    // Streets -> Apartment (north), Sector 7G (south)
    {2, 1, -1, -1, -1}    // Sector 7G -> Streets (north)
};

// NPC placements: {mapIndex, npcId, tileX, tileY, dialogId}
// Note: dialogId is stored as a string pointer, so the array element type is const char*
static const void* ARC1_NPC_PLACEMENT[][5] = {
    {(const void*)1, (const void*)0, (const void*)15, (const void*)20, "arc1_zero_meet"},   // ZERO on Neon Streets
    {(const void*)1, (const void*)1, (const void*)35, (const void*)10, "arc1_merchant"},     // Data Merchant
    {(const void*)2, (const void*)2, (const void*)10, (const void*)25, "arc1_terminal"}      // Corrupted Terminal
};

// Encounter zones: {mapIndex, tileX, tileY, width, height, encounterId}
static const uint16_t ARC1_ENCOUNTER_ZONES[][6] = {
    {1, 20, 15, 10, 10, 0},   // Streets: Data Wraiths
    {2, 5, 5, 15, 15, 1},     // Sector 7G: mixed encounters
    {2, 30, 20, 10, 10, 3}    // Sector 7G: boss encounter
};

#endif // ARC1_MAP_H