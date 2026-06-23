// ============================================================================
// NEON REQUIEM — Entry Point
// ============================================================================
// Arduino framework entry point for ESP32-S3.
// Initializes the game engine and enters the main loop.
// ============================================================================

#include <Arduino.h>
#include "config/config.h"
#include "engine/game.h"

// ============================================================================
// ARDUINO SETUP
// ============================================================================

void setup() {
    // Serial is initialized inside Game::run()
    // Start the game engine
    Game::getInstance().run();
}

// ============================================================================
// ARDUINO LOOP
// ============================================================================
// The game engine runs its own loop inside Game::run().
// This loop should never be reached under normal operation.
// If it is, we enter a safe idle state.

void loop() {
    // If we reach here, the game has exited its main loop
    // Enter low-power idle
    delay(1000);

    // Attempt to restart the game
    Game::getInstance().restart();
}