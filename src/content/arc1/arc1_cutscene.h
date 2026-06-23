/*
 * NEON REQUIEM - Arc 1: "The Signal"
 * Cutscene data: frame sequences for story cutscenes
 */

#ifndef ARC1_CUTSCENE_H
#define ARC1_CUTSCENE_H

#include <Arduino.h>
#include "config/config.h"

// Arc 1 cutscene: "The Signal Arrives"
// Frames: {duration, bgColor, effectType, effectIntensity, text}
static const uint16_t ARC1_CUTSCENE_SIGNAL[][5] = {
    {60, 0x0000, 0, 0},    // Black screen, 1 second
    {90, 0x0010, 1, 40},   // Fade in to dark blue with rain
    {120, 0x001F, 1, 80},  // KAI's apartment, heavy rain
    {60, 0x07E0, 2, 30},   // Terminal screen glows green
    {90, 0x001F, 1, 60},   // Back to room, signal received
    {120, 0x0000, 0, 0},   // Blackout transition
    {60, 0xF800, 2, 100},  // Red flash - danger
    {90, 0x001F, 1, 40},   // Rain in the streets
    {120, 0x0000, 0, 0}    // Fade to gameplay
};

// Arc 1 cutscene: "First Battle"
static const uint16_t ARC1_CUTSCENE_BATTLE[][5] = {
    {30, 0x0000, 0, 0},    // Quick black
    {60, 0xF800, 2, 80},   // Red alert flash
    {90, 0x001F, 1, 60},   // Sector 7G, rain
    {120, 0x0000, 0, 0},   // Black - battle start
    {60, 0xFFFF, 3, 50},   // White flash - impact
    {90, 0x001F, 1, 40},   // Aftermath, rain
    {60, 0x0000, 0, 0}     // Fade to gameplay
};

// Arc 1 cutscene: "Arc Complete"
static const uint16_t ARC1_CUTSCENE_COMPLETE[][5] = {
    {60, 0x0000, 0, 0},
    {90, 0x001F, 1, 40},
    {120, 0x0000, 0, 0},
    {180, 0x0000, 0, 0},   // "ARC 1 COMPLETE" text overlay
    {60, 0x0000, 0, 0}
};

#endif // ARC1_CUTSCENE_H