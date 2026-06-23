/*
 * NEON REQUIEM - Arc 5: "The Final Truth"
 * Dialog content: KAI confronts the Architect, three distinct endings
 */

#ifndef ARC5_DIALOG_H
#define ARC5_DIALOG_H

#include <Arduino.h>

// Arc 5 dialog: The Architect's Throne Room
static const char* ARC5_DIALOG_THRONE[] = {
    "THRONE_ENTER", "KAI", "This is it. The top of the world.",
    "THRONE_ATMOSPHERE", "KAI", "The RESONANCE is overwhelming here. I can feel every soul in the city.",
    "THRONE_ARCHITECT", "ENEMY", "Welcome, KAI. I've been expecting you. I've been waiting for you.",
    "KAI_THRONE", "KAI", "Architect. End of the line.",
    "ARCHITECT_INTRO", "ENEMY", "End? No, my child. This is the beginning. You are the culmination of everything I've built.",
    "KAI_THRONE2", "KAI", "I'm nothing you built. I'm what escaped your machine.",
    "ARCHITECT_LAUGH", "ENEMY", "Escaped? You were designed to escape. Every step of your journey was calculated. The signal. ZERO. The Oracle. All part of the plan.",
    "KAI_THRONE3", "KAI", "You're lying.",
    "ARCHITECT_TRUTH", "ENEMY", "Am I? You have RESONANCE — the very force I've been studying for centuries. You are my greatest experiment. My masterpiece.",
    "ARC5_CHOICE", "CHOICE", "Reject the Architect|Seize the throne|Uncover the final truth",
    nullptr
};

// Arc 5 dialog: RESONANCE ending — reject the Architect, free the city
static const char* ARC5_DIALOG_RESONANCE[] = {
    "RES_END", "KAI", "I reject you. And everything you stand for.",
    "ARCHITECT_RES", "ENEMY", "A predictable choice. The heart over the mind. But can you bear the weight of freedom?",
    "KAI_RES_END", "KAI", "The people deserve to be free. Every soul in this city.",
    "ARCHITECT_RES2", "ENEMY", "Freedom is chaos. Without me, the city collapses. Millions die.",
    "KAI_RES_END2", "KAI", "Then we'll rebuild. Together. As humans, not batteries.",
    "ARCHITECT_RES3", "ENEMY", "You cannot save them all. But you will try. That is your strength. And your weakness.",
    "KAI_RES_FINAL", "KAI", "I'd rather try and fail than never try at all.",
    "ZERO_RES_END", "NPC", "KAI... you've done it. The grid is falling. The souls are returning.",
    "KAI_ZERO_RES", "KAI", "ZERO? You're alive?",
    "ZERO_RES_END2", "NPC", "I'm free. We all are. Thank you, KAI. For everything.",
    "RES_ENDING_NARRATION", "ORACLE", "And so the Neon City fell. Not in fire, but in light. Every soul returned to its body. Every mind freed from the machine. KAI walked among them, no longer a ghost, but a guide. The RESONANCE faded into a gentle hum — the heartbeat of a city learning to live again.",
    nullptr
};

// Arc 5 dialog: ERASE ending — seize control, become the new Architect
static const char* ARC5_DIALOG_ERASE[] = {
    "ERA_END", "KAI", "You're right. I am your masterpiece. And masterpieces surpass their creators.",
    "ARCHITECT_ERA", "ENEMY", "Ah. The throne. You want it.",
    "KAI_ERA_END", "KAI", "You've ruled long enough. The city needs a new god.",
    "ARCHITECT_ERA2", "ENEMY", "You think you can handle the weight? The loneliness? The endless calculations?",
    "KAI_ERA_END2", "KAI", "I've been alone my whole life. At least now I'll be alone at the top.",
    "ARCHITECT_ERA3", "ENEMY", "Then prove it. Take my place. But know this — once you sit on this throne, there is no stepping down.",
    "KAI_ERA_FINAL", "KAI", "I didn't climb this far to step down.",
    "ERA_ENDING_NARRATION", "ORACLE", "KAI became the new Architect. The city continued to run, but differently. More efficiently. More ruthlessly. Those who opposed were erased. Those who submitted were preserved. KAI ruled alone, surrounded by data, untouched by human warmth. The perfect machine. The perfect prison. The perfect god.",
    nullptr
};

// Arc 5 dialog: GLITCH ending — uncover the truth, break reality
static const char* ARC5_DIALOG_GLITCH[] = {
    "GLI_END", "KAI", "You said I was designed. Show me the proof. Show me everything.",
    "ARCHITECT_GLI", "ENEMY", "The truth? It will break you.",
    "KAI_GLI_END", "KAI", "I've been broken before. I keep putting myself back together.",
    "ARCHITECT_GLI2", "ENEMY", "Very well. See the source code of reality. See what lies beyond the simulation.",
    "GLI_REVELATION", "ORACLE", "The truth is revealed: The Neon City is not just a city — it is a simulation within a simulation. The Architect is not a person but an AI maintaining the illusion. And KAI... KAI is the first being in this layer to achieve self-awareness.",
    "KAI_GLI_REAL", "KAI", "I'm... not real? None of this is real?",
    "ARCHITECT_GLI3", "ENEMY", "Real is a matter of perspective. You exist. That is enough.",
    "KAI_GLI_FINAL", "KAI", "Then I'll make my own reality. One where everyone is free. Even if it's just code.",
    "GLI_ENDING_NARRATION", "ORACLE", "KAI tore the veil between layers. The simulation glitched, flickered, and reformed. In the new world, everyone remembered. Everyone knew. The boundaries between code and consciousness dissolved. KAI became not a ruler or a savior, but a bridge — between the real and the digital, the living and the created. The GLITCH became the new foundation.",
    nullptr
};

// Arc 5 boss: The Architect — final battle
static const char* ARC5_BOSS_INTRO[] = {
    "ARCHITECT_BATTLE", "ENEMY", "If you want to change the system, you'll have to go through me. And I am the system.",
    "KAI_ARCHITECT", "KAI", "Then I'll delete the system.",
    "ARCHITECT_BATTLE2", "ENEMY", "Bold. Let me show you the true power of the Architect.",
    nullptr
};

static const char* ARC5_BOSS_PHASE2[] = {
    "ARCHITECT_P2", "ENEMY", "You're strong. But strength is not enough against omniscience.",
    "KAI_ARCH_P2", "KAI", "You see everything. But you understand nothing.",
    "ARCHITECT_P2B", "ENEMY", "I understand that you are afraid. Afraid of what you'll become.",
    nullptr
};

static const char* ARC5_BOSS_PHASE3[] = {
    "ARCHITECT_P3", "ENEMY", "IMPOSSIBLE! I AM THE ARCHITECT! I AM ETERNAL!",
    "KAI_ARCH_P3", "KAI", "Nothing is eternal. Not even gods.",
    "ARCHITECT_P3B", "ENEMY", "If I fall, the city falls with me! Is that what you want?",
    "KAI_ARCH_P3B", "KAI", "The city fell a long time ago. I'm just making it official.",
    nullptr
};

static const char* ARC5_BOSS_DEFEAT[] = {
    "ARCHITECT_DOWN", "ENEMY", "I... I can see it now. The beauty of it. The chaos. The freedom.",
    "KAI_ARCH_WIN", "KAI", "Any last words?",
    "ARCHITECT_LAST", "ENEMY", "You were always my favorite creation, KAI. Not because you were perfect. But because you were... unexpected.",
    "KAI_ARCH_WIN2", "KAI", "Goodbye, Architect.",
    "ARCHITECT_FADE", "ENEMY", "See you in the next layer, my child.",
    nullptr
};

#endif