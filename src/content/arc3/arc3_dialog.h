/*
 * NEON REQUIEM - Arc 3: "The Resistance"
 * Dialog content: KAI joins the underground resistance
 */

#ifndef ARC3_DIALOG_H
#define ARC3_DIALOG_H
#include <Arduino.h>

static const char* ARC3_DIALOG_RESISTANCE[] = {
    "RES_LEADER", "NPC", "You're the one ZERO told us about. The one with RESONANCE.",
    "KAI_RES", "KAI", "I've seen what the city hides. The data core. The truth about the people.",
    "RES_LEADER2", "NPC", "Then you know why we fight. The city drains us. Our memories, our identities, our very souls.",
    "KAI_RES2", "KAI", "How do we stop it?",
    "RES_PLAN", "NPC", "There's a central server in the Admin Spire. Destroy it, and the grid falls. The people go free.",
    "KAI_RES3", "KAI", "That's suicide. The Spire is the most guarded place in the city.",
    "RES_PLAN2", "NPC", "We have a way in. But we need someone who can navigate the digital layers. Someone with RESONANCE.",
    "ARC3_CHOICE", "CHOICE", "Join the resistance|Use them for your own goals|Infiltrate alone",
    nullptr
};

static const char* ARC3_DIALOG_BETRAYAL[] = {
    "BETRAY_INTRO", "NPC", "KAI... we've received word. The city knows about our plan.",
    "KAI_BETRAY", "KAI", "How? We've been careful.",
    "BETRAY_REVEAL", "NPC", "Someone sold us out. The Admin Spire is expecting us.",
    "KAI_BETRAY2", "KAI", "We go anyway. We don't have another choice.",
    nullptr
};

static const char* ARC3_BOSS_INTRO[] = {
    "COMMANDER_APPEAR", "ENEMY", "So the little rat finally surfaces. I've been tracking you since Sector 7G.",
    "KAI_COMMANDER", "KAI", "Who are you?",
    "COMMANDER_INTRO", "ENEMY", "Commander Vex. Head of City Security. And your death sentence.",
    "VEX_THREAT", "ENEMY", "You've seen too much. Heard too much. Time to be erased.",
    nullptr
};

static const char* ARC3_BOSS_DEFEAT[] = {
    "VEX_DOWN", "ENEMY", "Impossible... I am the city's will...",
    "KAI_VEX", "KAI", "The city's will ends here.",
    "RES_LEADER3", "NPC", "The path to the Spire is open. But Vex was just the first line of defense.",
    nullptr
};
#endif