/*
 * NEON REQUIEM - Arc 4: "The Admin Spire"
 * Dialog content: KAI ascends the Admin Spire, confronts the Architect's inner sanctum
 */

#ifndef ARC4_DIALOG_H
#define ARC4_DIALOG_H

#include <Arduino.h>

// Arc 4 dialog: Entering the Spire
static const char* ARC4_DIALOG_ENTRY[] = {
    "SPIRE_GATE", "NPC", "The Admin Spire. No one who enters ever comes back.",
    "KAI_SPIRE", "KAI", "Then I'll be the first. The answers are at the top.",
    "SPIRE_WARNING", "NPC", "The Architect's personal guard awaits. These aren't city security — they're something worse.",
    "KAI_SPIRE2", "KAI", "I've faced worse. Stay back. I'll clear a path.",
    nullptr
};

// Arc 4 dialog: The Ascent — floor by floor revelations
static const char* ARC4_DIALOG_ASCENT[] = {
    "ASCENT_1", "KAI", "Floor after floor of data. Memories. Lives reduced to numbers.",
    "ASCENT_2", "KAI", "These terminals... they're processing human consciousness. Converting people into code.",
    "ASCENT_3", "KAI", "The city isn't just a city. It's a machine. And we're the fuel.",
    "ASCENT_4", "KAI", "I can feel the RESONANCE getting stronger. Something's pulling me upward.",
    nullptr
};

// Arc 4 dialog: The Architect's Logs — lore revelations
static const char* ARC4_DIALOG_LOGS[] = {
    "LOG_1", "ORACLE", "Architect Log 001: The Resonance Project is approved. We will digitize the human soul.",
    "LOG_2", "ORACLE", "Architect Log 047: Phase 2 complete. Subjects retain consciousness but lose physical form. Moral concerns noted and dismissed.",
    "LOG_3", "ORACLE", "Architect Log 128: The city grows. More souls, more power. The grid is self-sustaining.",
    "LOG_4", "ORACLE", "Architect Log 255: ZERO has escaped. The prototype with free will. This cannot be permitted.",
    "LOG_5", "ORACLE", "Architect Log 300: KAI has awakened. The failsafe protocol. Or perhaps... the evolution I always hoped for.",
    nullptr
};

// Arc 4 dialog: Route-specific encounters
static const char* ARC4_DIALOG_RESONANCE[] = {
    "RES_PATH", "ORACLE", "You chose compassion. The hard road. Every soul you've saved has strengthened your RESONANCE.",
    "KAI_RES_PATH", "KAI", "I couldn't leave them behind. They're people, not data.",
    "RES_PATH2", "ORACLE", "The Architect fears you most of all. A heart that cannot be corrupted. Go. Break the cycle.",
    nullptr
};

static const char* ARC4_DIALOG_ERASE[] = {
    "ERA_PATH", "ORACLE", "You chose power. Every enemy eliminated. Every obstacle destroyed.",
    "KAI_ERA_PATH", "KAI", "They stood in my way. They made their choice.",
    "ERA_PATH2", "ORACLE", "The Architect respects your ruthlessness. But respect is not surrender. Prove you are worthy of the throne.",
    nullptr
};

static const char* ARC4_DIALOG_GLITCH[] = {
    "GLI_PATH", "ORACLE", "You chose truth. You've seen behind every curtain, every lie.",
    "KAI_GLI_PATH", "KAI", "The city is built on a foundation of deception. I want to see the source code.",
    "GLI_PATH2", "ORACLE", "The Architect's greatest secret awaits. But some truths... once seen, cannot be unseen.",
    nullptr
};

// Arc 4 boss: The Admin Avatar — Architect's digital projection
static const char* ARC4_BOSS_INTRO[] = {
    "ADMIN_APPEAR", "ENEMY", "So. You've climbed my spire. Killed my servants. Read my logs.",
    "KAI_ADMIN", "KAI", "Show yourself, Architect. I know you're watching.",
    "ADMIN_INTRO", "ENEMY", "Watching? I'm everywhere. I am the city. I am the grid. I am the god of this machine.",
    "ADMIN_THREAT", "ENEMY", "This is my Avatar. A fraction of my will. And more than enough to erase you from existence.",
    "KAI_ADMIN2", "KAI", "I've been erased before. I'm still here.",
    "ADMIN_FINAL", "ENEMY", "Bold words for a ghost in my machine. Let me show you what happens to faulty code.",
    nullptr
};

static const char* ARC4_BOSS_PHASE2[] = {
    "ADMIN_P2", "ENEMY", "Impressive. You've adapted. But I've been optimizing for centuries.",
    "KAI_ADMIN_P2", "KAI", "You've been hiding. There's a difference.",
    "ADMIN_P2B", "ENEMY", "The RESONANCE in you... it's familiar. You're more like me than you know.",
    nullptr
};

static const char* ARC4_BOSS_PHASE3[] = {
    "ADMIN_P3", "ENEMY", "ENOUGH! I will not be denied by a malfunctioning variable!",
    "KAI_ADMIN_P3", "KAI", "I'm not a bug. I'm the patch.",
    "ADMIN_P3B", "ENEMY", "This body is temporary. I am eternal. You cannot kill an idea.",
    nullptr
};

static const char* ARC4_BOSS_DEFEAT[] = {
    "ADMIN_DOWN", "ENEMY", "You... you've destroyed the Avatar. But the real me... is beyond your reach.",
    "KAI_ADMIN_WIN", "KAI", "Where are you? I'm coming for you next.",
    "ADMIN_FINAL2", "ENEMY", "The top floor. The penthouse of reality. If you dare.",
    "KAI_ADMIN_WIN2", "KAI", "I dare.",
    nullptr
};

#endif