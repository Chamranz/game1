# NEON REQUIEM

**A Cyberpunk Noir RPG for ESP32-S3** — In the style of Undertale, with color pixel art, bullet-hell combat, and branching narrative.

> *"In a city where souls are data and memories are currency, one glitch in the system will change everything."*

---

## Table of Contents

- [Overview](#overview)
- [Hardware Requirements](#hardware-requirements)
- [Pin Configuration](#pin-configuration)
- [Project Structure](#project-structure)
- [Building](#building)
- [Story & Routes](#story--routes)
- [Game Systems](#game-systems)
- [Content Architecture](#content-architecture)
- [Memory Map](#memory-map)
- [SD Card Layout](#sd-card-layout)
- [License](#license)

---

## Overview

NEON REQUIEM is a portable RPG built for the ESP32-S3 microcontroller with color TFT display. It features:

- **320×240 16-bit color pixel art** via ST7789 TFT (80MHz SPI)
- **I2S audio** via MAX98357A amplifier (44.1kHz, 16-bit)
- **D-Pad + 4 button controls** with software debounce and auto-repeat
- **SD card storage** for sprites, music, maps, and save data
- **5 story arcs** across 3 branching routes (RESONANCE / ERASE / GLITCH)
- **Bullet-hell combat** with SOUL box, pattern memorization, and COUNTER-HACK parry
- **Overworld exploration** with NPCs, quests, and stealth mechanics
- **RESONANCE system** — interact with the digital soul of the city
- **Cutscenes** with parallax, rain effects, and glitch transitions

### Technical Specs

| Component | Specification |
|-----------|---------------|
| MCU | ESP32-S3-WROOM N16R8 |
| Flash | 16MB (QIO) |
| PSRAM | 8MB |
| Display | ST7789 320×240 TFT (SPI, 80MHz) |
| Audio | MAX98357A I2S Amplifier |
| Storage | MicroSD via SPI |
| Framerate | 30 FPS (double-buffered) |
| Framework | Arduino (PlatformIO) |

---

## Hardware Requirements

### Bill of Materials

| Component | Model | Quantity |
|-----------|-------|----------|
| Microcontroller | ESP32-S3-WROOM N16R8 Dev Board | 1 |
| Display | 2.0" ST7789 320×240 TFT (SPI) | 1 |
| Audio Amp | MAX98357A I2S Breakout | 1 |
| Speaker | 3W 4Ω / 8Ω | 1 |
| SD Card | MicroSD Card Module (SPI) | 1 |
| SD Card | 16GB+ Class 10 (FAT32) | 1 |
| D-Pad | Tactile switch 4-direction module | 1 |
| Buttons | Tactile switches (6×6mm) | 4 |
| Battery | LiPo 3.7V 2000mAh (with protection) | 1 |
| Power Mgmt | TP4056 Charging Module | 1 |
| Boost Conv | MT3608 (to 5V) | 1 |
| PCB | Custom or prototyping board | 1 |
| Misc | Resistors, capacitors, headers, wires | - |

---

## Pin Configuration

| Function | Pin | Notes |
|----------|-----|-------|
| **TFT MOSI** | GPIO 11 | SPI Data |
| **TFT SCLK** | GPIO 12 | SPI Clock |
| **TFT CS** | GPIO 10 | Chip Select |
| **TFT DC** | GPIO 9 | Data/Command |
| **TFT RST** | GPIO 8 | Reset |
| **TFT BL** | GPIO 7 | Backlight (PWM) |
| **D-Pad UP** | GPIO 1 | Pull-up |
| **D-Pad DOWN** | GPIO 2 | Pull-up |
| **D-Pad LEFT** | GPIO 3 | Pull-up |
| **D-Pad RIGHT** | GPIO 4 | Pull-up |
| **Button A** | GPIO 5 | Confirm/Interact |
| **Button B** | GPIO 6 | Cancel/Back |
| **Button START** | GPIO 15 | Menu |
| **Button SELECT** | GPIO 16 | Special |
| **I2S BCLK** | GPIO 17 | Bit Clock |
| **I2S LRC** | GPIO 18 | Left/Right Clock |
| **I2S DOUT** | GPIO 14 | Data Out |
| **SD MOSI** | GPIO 11 | Shared with TFT |
| **SD MISO** | GPIO 13 | SPI MISO |
| **SD SCLK** | GPIO 12 | Shared with TFT |
| **SD CS** | GPIO 21 | Chip Select |
| **BATTERY** | GPIO 4 | ADC (via voltage divider) |
| **POWER_HOLD** | GPIO 48 | Keep power on |

> **Note:** SD and TFT share SPI bus (MOSI=11, SCLK=12). They use separate CS pins.

---

## Project Structure

```
neon_requiem/
├── main.cpp                          # Arduino entry point
├── platformio.ini                    # PlatformIO configuration
├── partitions.csv                    # 16MB flash partition table
├── README.md                         # This file
│
├── src/
│   ├── config/
│   │   ├── config.h                  # Master configuration
│   │   ├── pins.h                    # Pin definitions
│   │   └── palette.h                 # Color palette & utilities
│   │
│   ├── core/
│   │   ├── input/
│   │   │   ├── input_manager.h       # GPIO input with debounce
│   │   │   └── input_manager.cpp
│   │   ├── display/
│   │   │   ├── display_manager.h     # PSRAM framebuffer, drawing primitives
│   │   │   └── display_manager.cpp
│   │   ├── audio/
│   │   │   ├── audio_manager.h       # I2S 4-channel mixer
│   │   │   └── audio_manager.cpp
│   │   └── storage/
│   │       ├── sd_manager.h          # SD card with PSRAM cache
│   │       └── sd_manager.cpp
│   │
│   ├── engine/
│   │   ├── game.h                    # Game engine singleton
│   │   ├── game.cpp                  # Main loop, state management
│   │   ├── scene_manager.h           # Scene stack (depth 8)
│   │   ├── scene_manager.cpp
│   │   ├── player.h                  # Player (KAI) state
│   │   └── player.cpp
│   │
│   ├── systems/
│   │   ├── dialog/
│   │   │   ├── dialog_system.h       # Dialog engine with branching
│   │   │   ├── dialog_system.cpp
│   │   │   ├── dialog_box.h          # Dialog box UI component
│   │   │   └── dialog_box.cpp
│   │   ├── battle/
│   │   │   ├── battle_system.h       # Battle engine
│   │   │   ├── battle_system.cpp
│   │   │   ├── soul_box.h            # Bullet-hell SOUL movement
│   │   │   ├── soul_box.cpp
│   │   │   ├── battle_react.h        # Attack pattern generators
│   │   │   ├── battle_react.cpp
│   │   │   ├── battle_pattern.h      # Pattern memorization minigame
│   │   │   └── battle_pattern.cpp
│   │   ├── cutscene/
│   │   │   ├── cutscene_system.h     # Cutscene engine
│   │   │   ├── cutscene_system.cpp
│   │   │   ├── cutscene_player.h     # Cutscene sprite player
│   │   │   └── cutscene_player.cpp
│   │   ├── overworld/
│   │   │   ├── overworld_scene.h     # Overworld exploration
│   │   │   ├── overworld_scene.cpp
│   │   │   ├── npc.h                 # NPC system
│   │   │   └── npc.cpp
│   │   ├── resonance/
│   │   │   ├── resonance_system.h    # RESONANCE mode
│   │   │   └── resonance_system.cpp
│   │   ├── quest/
│   │   │   ├── quest_manager.h       # Quest tracking
│   │   │   └── quest_manager.cpp
│   │   ├── route/
│   │   │   ├── route_manager.h       # Route alignment tracking
│   │   │   └── route_manager.cpp
│   │   ├── save/
│   │   │   ├── save_system.h         # Save/load with CRC
│   │   │   └── save_system.cpp
│   │   └── inventory/
│   │       ├── inventory.h           # Item management
│   │       └── inventory.cpp
│   │
│   ├── rendering/
│   │   ├── sprite_renderer.h         # Sprite cache & batch rendering
│   │   ├── sprite_renderer.cpp
│   │   ├── tilemap_renderer.h        # Tilemap with 4 layers
│   │   ├── tilemap_renderer.cpp
│   │   ├── effects/
│   │   │   ├── rain_effect.h         # Rain particle system
│   │   │   ├── rain_effect.cpp
│   │   │   ├── glitch_effect.h       # Glitch distortion
│   │   │   ├── glitch_effect.cpp
│   │   │   ├── crt_effect.h          # CRT scanlines & vignette
│   │   │   └── crt_effect.cpp
│   │   ├── ui/
│   │   │   ├── hud.h                 # HP bar, energy, minimap
│   │   │   ├── hud.cpp
│   │   │   ├── dialog_ui.h           # Dialog box rendering
│   │   │   ├── dialog_ui.cpp
│   │   │   ├── menu_ui.h             # Menu system (7 types)
│   │   │   └── menu_ui.cpp
│   │   ├── transitions/
│   │   │   ├── fade_transition.h     # 9 transition types
│   │   │   └── fade_transition.cpp
│   │   └── animation/
│   │       ├── sprite_animation.h    # Sprite animation clips
│   │       └── sprite_animation.cpp
│   │
│   └── content/
│       ├── content_manager.h         # Central content hub
│       ├── arc1/
│       │   ├── arc1_dialog.h         # Arc 1 dialog data
│       │   ├── arc1_battle.h         # Arc 1 battle data
│       │   ├── arc1_cutscene.h       # Arc 1 cutscene data
│       │   ├── arc1_map.h            # Arc 1 map data
│       │   └── arc1_quest.h          # Arc 1 quest data
│       ├── arc2/
│       │   ├── arc2_dialog.h         # Arc 2 dialog data
│       │   └── arc2_content.h        # Arc 2 battle/cutscene/map/quest
│       ├── arc3/
│       │   ├── arc3_dialog.h         # Arc 3 dialog data
│       │   └── arc3_content.h        # Arc 3 battle/cutscene/map/quest
│       ├── arc4/
│       │   ├── arc4_dialog.h         # Arc 4 dialog data
│       │   └── arc4_content.h        # Arc 4 battle/cutscene/map/quest
│       └── arc5/
│           ├── arc5_dialog.h         # Arc 5 dialog data
│           └── arc5_content.h        # Arc 5 battle/cutscene/map/quest
│
└── tools/
    └── post_build.py                 # Post-build firmware copy & metadata
```

---

## Building

### Prerequisites

1. Install [PlatformIO](https://platformio.org/) (VS Code extension or CLI)
2. Install required libraries (auto-resolved by PlatformIO):
   - `TFT_eSPI` by Bodmer
   - `ArduinoJson` by Benoit Blanchon
   - `ESP8266Audio` by Earle F. Philhower

### Build & Upload

```bash
# Build
pio run

# Build and upload to ESP32-S3
pio run --target upload

# Monitor serial output
pio device monitor

# Clean build
pio run --target clean
```

### Configuration

Edit [`src/config/config.h`](src/config/config.h) to adjust:
- `DEBUG_LEVEL` — 0 (silent) to 4 (verbose)
- `TARGET_FPS` — framerate target
- `FEATURE_*` — enable/disable subsystems
- PSRAM allocation limits

---

## Story & Routes

### Arc 1: "The Signal" — Awakening

KAI awakens in a neon-drenched apartment with no memories. A mysterious signal leads to ZERO, a rogue AI who reveals the truth: the city is a machine that consumes human souls.

**Boss:** City Hound — A corrupted security construct

### Arc 2: "The Data Heart" — Revelation

KAI delves into the city's central data core. The Oracle reveals the three paths: RESONANCE (redemption), ERASE (power), GLITCH (truth).

**Boss:** Core Sentinel — Guardian of the city's archives

### Arc 3: "The Resistance" — Alliance

KAI joins the underground resistance. Betrayal strikes as Commander Vex corners the group. Route choices begin to crystallize.

**Boss:** Commander Vex — Head of City Security

### Arc 4: "The Admin Spire" — Ascent

KAI ascends the Admin Spire, confronting the Architect's digital projection. The route determines the encounters and revelations.

**Boss:** Admin Avatar — The Architect's will made manifest

### Arc 5: "The Final Truth" — Resolution

KAI confronts the Architect in the throne room. Three distinct endings based on the chosen route:

| Route | Ending | Theme |
|-------|--------|-------|
| **RESONANCE** | The city falls, souls return, KAI becomes a guide | Redemption |
| **ERASE** | KAI becomes the new Architect, rules alone | Power |
| **GLITCH** | Reality breaks, KAI becomes a bridge between worlds | Truth |

---

## Game Systems

### Combat System

- **4 actions:** HACK (attack), SCAN (analyze, increase damage), LINK (heal/boost), MERCY (spare)
- **2 attack types:** REACTION (bullet-hell dodge), PATTERN (button sequence memorization)
- **COUNTER-HACK:** Parry mechanic with 200ms window
- **Boss phases:** 3-5 phases per boss with escalating patterns

### Dialog System

- JSON-driven branching dialog with typewriter effect
- 12 speaker types with unique colors
- Conditional branches based on route, quest status, and flags
- Choice menus with up to 4 options

### Overworld

- Tile-based maps with 4 layers (background, objects, upper, collision)
- Camera follows player with bounds clamping
- NPC interaction, quest triggers, encounter zones
- Stealth mechanics (sneak past enemies)

### RESONANCE Mode

- Interact with the digital soul of the city
- Reveal hidden objects, memories, and pathways
- Pulsing highlight effect (sin-based animation)
- 16 object slots per map

### Visual Effects

- **Rain:** 120 drops, 3 parallax layers, configurable intensity
- **Glitch:** 5 modes (scan lines, channel offset, block corrupt, full)
- **CRT:** Scanlines, vignette LUT, phosphor glow, curvature

---

## Content Architecture

Content is organized by arc in compact multi-header format:

```
arcX/
├── arcX_dialog.h     # Dialog arrays (nullptr-terminated)
└── arcX_content.h    # Battle, cutscene, map, quest data
```

The [`ContentManager`](src/content/content_manager.h) singleton provides unified access to all arc data via string IDs and arc numbers.

### Data Formats

**Dialog:** `{id, speaker, text, ...}` arrays terminated by `nullptr`
**Battle:** Enemy name arrays, stat tables, boss phase definitions
**Cutscene:** `{duration, bgColor, effectType, effectIntensity}` frame arrays
**Map:** Map dimensions, connections, NPC placements
**Quest:** Quest definitions, objectives, rewards

---

## Memory Map

| Region | Size | Contents |
|--------|------|----------|
| **PSRAM Sprites** | 3 MB | Sprite sheets, frame data |
| **PSRAM Audio** | 2 MB | WAV samples, music buffers |
| **PSRAM Maps** | 1 MB | Tilemap data |
| **PSRAM Dialogs** | 512 KB | Dialog JSON |
| **PSRAM UI** | 256 KB | UI elements |
| **PSRAM Effects** | 512 KB | Particle data, effect buffers |
| **PSRAM Reserve** | ~720 KB | Runtime allocations |
| **Flash (app0)** | 2 MB | Firmware |
| **Flash (app1)** | 2 MB | OTA update |
| **Flash (spiffs)** | 8 MB | Game content (fallback) |

---

## SD Card Layout

```
/sd/
├── sprites/          # .spr sprite files
├── music/            # .wav music tracks
├── sfx/              # .wav sound effects
├── maps/             # .json map files
├── dialogs/          # .json dialog files
├── fonts/            # .fnt font files
└── saves/            # .sav save files
```

---

## License

All rights reserved. NEON REQUIEM is a fictional game project.

---

*Built with PlatformIO, Arduino framework, and a lot of neon.*