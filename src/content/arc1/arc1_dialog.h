/*
 * NEON REQUIEM - Arc 1: "The Signal"
 * Dialog content for the first story arc
 * KAI receives a mysterious signal and enters the Neon Grid
 */

#ifndef ARC1_DIALOG_H
#define ARC1_DIALOG_H

#include <Arduino.h>
#include "config/config.h"

// Arc 1 dialog nodes
// Each node: id, speaker, text, type, nextNode, choices[]

// Prologue: The Signal
static const char* ARC1_DIALOG_INTRO[] = {
    "SYS_BOOT", "SYS", "Neural interface boot sequence initiated... v2.4.1",
    "SYS_BOOT2", "SYS", "Welcome back, KAI. System integrity: 97.4%%",
    "KAI_WAKE", "KAI", "Ugh... another cycle. Another shift in the data mines.",
    "KAI_ROOM", "KAI", "My terminal's blinking. That's... not normal. It's a raw signal, no routing.",
    "SYS_ALERT", "SYS", "WARNING: Unauthorized transmission detected. Source: UNKNOWN.",
    "KAI_CURIOUS", "KAI", "Unknown? Nothing gets past the city's firewalls. Unless...",
    "SYS_TRACE", "SYS", "Signal origin traced to: Sector 7G. Abandoned district. Fatality risk: HIGH.",
    "KAI_DECIDE", "KAI", "Sector 7G? That's been dead for years. But this signal... it's calling to something. Someone's out there.",
    "ARC1_INTRO_END", "NARRATOR", "And so KAI steps out into the rain-soaked streets of the Neon City, following a ghost signal into the unknown.",
    nullptr
};

// First NPC encounter: ZERO
static const char* ARC1_DIALOG_ZERO_MEET[] = {
    "ZERO_APPEAR", "ZERO", "Hey. You're the one who picked up the signal.",
    "KAI_SURPRISE", "KAI", "Whoa! Who are you? How do you know about that?",
    "ZERO_INTRO", "ZERO", "Name's ZERO. I've been watching you, KAI. That signal? I sent it.",
    "KAI_CONFUSED", "KAI", "You sent it? But the trace said Sector 7G...",
    "ZERO_EXPLAIN", "ZERO", "I routed it through the dead sector to avoid surveillance. The city's eyes are everywhere. But you... you have RESONANCE. You can see what others can't.",
    "KAI_RESONANCE", "KAI", "RESONANCE? What are you talking about?",
    "ZERO_OFFER", "ZERO", "Come with me. I'll show you the truth behind the Neon City. Or don't. But the signal's just the beginning.",
    "ARC1_ZERO_CHOICE", "CHOICE", "Follow ZERO|Investigate alone|Report to authorities",
    nullptr
};

// Choice follow-up: Follow ZERO
static const char* ARC1_DIALOG_FOLLOW_ZERO[] = {
    "FOLLOW_YES", "KAI", "Alright, ZERO. Show me what this is about.",
    "ZERO_LEAD", "ZERO", "Good choice. Stay close and keep your eyes open. The city's about to show you its real face.",
    "FOLLOW_START", "NARRATOR", "KAI follows ZERO into the underbelly of the Neon City. The RESONANCE route begins to take shape.",
    nullptr
};

// Choice follow-up: Investigate alone
static const char* ARC1_DIALOG_ALONE[] = {
    "ALONE_YES", "KAI", "I work alone. If there's something to find, I'll find it myself.",
    "ZERO_WARN", "ZERO", "Suit yourself. But when the city's hounds come for you, don't say I didn't offer.",
    "ALONE_START", "NARRATOR", "KAI strikes out on their own path. The ERASE route begins to crystallize.",
    nullptr
};

// Choice follow-up: Report to authorities
static const char* ARC1_DIALOG_REPORT[] = {
    "REPORT_YES", "KAI", "This should go through proper channels. I'm reporting this.",
    "ZERO_LAUGH", "ZERO", "Heh. You think the authorities don't already know? You're walking right into their game.",
    "REPORT_START", "NARRATOR", "KAI chooses the path of order. But in the Neon City, order is just another illusion. The GLITCH route flickers to life.",
    nullptr
};

// Arc 1 boss encounter: The Hound
static const char* ARC1_DIALOG_BOSS_INTRO[] = {
    "HOUND_APPEAR", "ENEMY", "INTRUDER DETECTED. UNAUTHORIZED ACCESS TO SECTOR 7G.",
    "KAI_HOUND", "KAI", "What is that thing?",
    "ZERO_HOUND", "ZERO", "City Hound. Autonomous security unit. They've been tracking us since we entered the sector.",
    "HOUND_THREAT", "ENEMY", "SURRENDER FOR PROCESSING. RESISTANCE WILL BE MET WITH FORCE.",
    "KAI_HOUND_READY", "KAI", "I'm not going down without a fight!",
    "ARC1_BOSS_START", "NARRATOR", "The City Hound powers up its weapons. The first true battle begins.",
    nullptr
};

// Arc 1 boss defeat
static const char* ARC1_DIALOG_BOSS_DEFEAT[] = {
    "HOUND_DOWN", "ENEMY", "SYSTEM FAILURE... SIGNAL TRANSMITTED TO CENTRAL CORE...",
    "KAI_VICTORY", "KAI", "Is it... dead?",
    "ZERO_VICTORY", "ZERO", "Disabled. But it sent a ping. Central knows we're here now. We need to move.",
    "KAI_AFTERMATH", "KAI", "What was that signal you showed me? The one hidden in the data stream?",
    "ZERO_REVEAL", "ZERO", "Proof. Evidence of what the city does to those who don't comply. People who've been... erased.",
    "ARC1_END", "NARRATOR", "Arc 1 complete. KAI has taken the first step into a deeper conspiracy. The Neon City's secrets begin to unravel.",
    nullptr
};

#endif // ARC1_DIALOG_H