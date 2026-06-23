/*
 * NEON REQUIEM - Arc 2: "The Data Heart"
 * Dialog content: KAI delves into the city's central data core
 */

#ifndef ARC2_DIALOG_H
#define ARC2_DIALOG_H

#include <Arduino.h>

// Arc 2 dialog: Meeting the Oracle
static const char* ARC2_DIALOG_ORACLE[] = {
    "ORACLE_APPEAR", "ORACLE", "You've done well to reach the Data Heart, little spark.",
    "KAI_ORACLE", "KAI", "Who are you? Another ghost in the machine?",
    "ORACLE_INTRO", "ORACLE", "I am the Oracle. I've been watching since before the city was built. ZERO's signal was meant to find you.",
    "KAI_ORACLE2", "KAI", "ZERO works for you?",
    "ORACLE_TRUTH", "ORACLE", "ZERO works for the truth. As will you, once you understand what's at stake.",
    "ORACLE_VISION", "ORACLE", "The Neon City is built on a lie. Every soul here is a battery in a machine they don't know exists. You can tear it down. Or you can become its master.",
    "ARC2_CHOICE", "CHOICE", "Help the people|Seize control|Uncover the truth",
    nullptr
};

static const char* ARC2_DIALOG_HELP[] = {
    "HELP_YES", "KAI", "I'll help the people. They deserve to know the truth.",
    "ORACLE_HELP", "ORACLE", "Then your path is RESONANCE. You'll need allies. Start with the underground resistance in Sector 4.",
    nullptr
};

static const char* ARC2_DIALOG_CONTROL[] = {
    "CONTROL_YES", "KAI", "Master of the city? That sounds more useful than truth.",
    "ORACLE_CONTROL", "ORACLE", "The ERASE route. Power without conscience. You'll need to eliminate those who stand in your way.",
    nullptr
};

static const char* ARC2_DIALOG_TRUTH[] = {
    "TRUTH_YES", "KAI", "I want to see what's really going on. All of it.",
    "ORACLE_TRUTH2", "ORACLE", "The GLITCH route. You'll see behind every curtain. But some truths are dangerous.",
    nullptr
};

// Arc 2 boss: The Core Sentinel
static const char* ARC2_BOSS_INTRO[] = {
    "SENTINEL_APPEAR", "ENEMY", "UNAUTHORIZED ACCESS TO CORE ARCHIVES. SECURITY PROTOCOL OMEGA ACTIVATED.",
    "KAI_SENTINEL", "KAI", "Another Hound? No... this one's different.",
    "ORACLE_SENTINEL", "ORACLE", "The Core Sentinel. It guards the city's central data. You must defeat it to proceed.",
    "SENTINEL_THREAT", "ENEMY", "INTRUDERS WILL BE PURGED. ALL DATA WILL BE PROTECTED.",
    nullptr
};

static const char* ARC2_BOSS_DEFEAT[] = {
    "SENTINEL_DOWN", "ENEMY", "CORE BREACH DETECTED... TRANSMITTING FINAL LOG...",
    "KAI_SENTINEL2", "KAI", "It's down. The core is open.",
    "ORACLE_REVEAL", "ORACLE", "Now you'll see the truth. The city's original sin. The day they decided who gets to be human.",
    nullptr
};

#endif