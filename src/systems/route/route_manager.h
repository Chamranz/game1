/*
 * NEON REQUIEM - Route Manager
 * Handles game route tracking, branching logic, and route-specific content
 * Three routes: RESONANCE (redemption), ERASE (power), GLITCH (truth)
 */

#ifndef ROUTE_MANAGER_H
#define ROUTE_MANAGER_H

#include <Arduino.h>
#include "config/config.h"

// Maximum route-related constants
#define MAX_ROUTE_FLAGS 32
#define MAX_ROUTE_EVENTS 64
#define MAX_ROUTE_DECISIONS 48
#define ROUTE_LOCK_THRESHOLD 3  // Number of route-specific decisions to lock a route

// Route lock states
enum RouteLockState : uint8_t {
    ROUTE_UNLOCKED = 0,      // All routes still available
    ROUTE_LOCKED_IN = 1,     // Route locked in (irreversible)
    ROUTE_LOCKED_OUT = 2     // Route locked out (no longer accessible)
};

// Route decision impact
enum DecisionImpact : uint8_t {
    IMPACT_NONE = 0,
    IMPACT_RESONANCE_POS = 1,  // Pushes toward RESONANCE route
    IMPACT_RESONANCE_NEG = 2,  // Pushes away from RESONANCE
    IMPACT_ERASE_POS = 3,      // Pushes toward ERASE route
    IMPACT_ERASE_NEG = 4,      // Pushes away from ERASE
    IMPACT_GLITCH_POS = 5,     // Pushes toward GLITCH route
    IMPACT_GLITCH_NEG = 6,     // Pushes away from GLITCH
    IMPACT_CRITICAL = 7        // Critical decision (counts double)
};

// Route event types
enum RouteEventType : uint8_t {
    EVENT_DECISION = 0,       // Player made a moral choice
    EVENT_QUEST_COMPLETE = 1, // Quest completed
    EVENT_NPC_ENCOUNTER = 2,  // Key NPC interaction
    EVENT_ITEM_USED = 3,      // Key item usage
    EVENT_BATTLE_RESULT = 4,  // Battle outcome (spare/kill)
    EVENT_CUTSCENE_SEEN = 5,  // Cutscene viewed
    EVENT_LOCATION_ENTER = 6, // Location entered
    EVENT_SECRET_FOUND = 7    // Secret discovered
};

// Route flag structure
struct RouteFlag {
    char id[24];              // Flag identifier
    bool value;               // Flag state
    uint8_t arcId;            // Which arc this flag belongs to
};

// Route decision record
struct RouteDecision {
    char id[24];              // Decision identifier
    DecisionImpact impact;    // How this affects route alignment
    uint8_t arcId;            // Which arc this decision was in
    uint16_t timestamp;       // Game time when decision was made
    bool critical;            // Was this a critical decision?
};

// Route event record
struct RouteEvent {
    RouteEventType type;      // Event type
    char id[24];              // Event identifier
    uint8_t arcId;            // Which arc this event occurred in
    uint16_t timestamp;       // Game time when event occurred
};

// Route alignment scores (determine which route the player is on)
struct RouteAlignment {
    int8_t resonance;         // -100 to +100, RESONANCE route alignment
    int8_t erase;             // -100 to +100, ERASE route alignment
    int8_t glitch;            // -100 to +100, GLITCH route alignment
    
    // Dominant route based on current alignment
    GameRoute getDominantRoute() const;
    
    // Is a route locked in?
    bool isRouteLocked(GameRoute route) const;
};

class RouteManager {
public:
    // Singleton access
    static RouteManager& getInstance();
    
    // Initialization
    void begin();
    void reset();
    
    // Route alignment management
    void addDecisionImpact(const char* decisionId, DecisionImpact impact, uint8_t arcId, bool critical = false);
    int8_t getRouteAlignment(GameRoute route) const;
    RouteAlignment getAlignment() const { return m_alignment; }
    
    // Route locking
    bool isRouteLocked(GameRoute route) const;
    GameRoute getLockedRoute() const { return m_lockedRoute; }
    RouteLockState getRouteLockState(GameRoute route) const;
    bool lockRoute(GameRoute route);  // Force-lock a route
    
    // Route determination
    GameRoute determineRoute() const;  // Calculate current route based on alignment
    GameRoute getCurrentRoute() const { return m_currentRoute; }
    void updateRoute();  // Recalculate route based on alignment
    
    // Route flags (for content gating)
    void setFlag(const char* flagId, bool value, uint8_t arcId);
    bool getFlag(const char* flagId) const;
    void clearFlags(uint8_t arcId);  // Clear all flags for an arc
    void clearAllFlags();
    
    // Route events
    void recordEvent(RouteEventType type, const char* eventId, uint8_t arcId);
    bool hasEventOccurred(const char* eventId) const;
    int getEventCount(RouteEventType type) const;
    
    // Route-specific content access
    bool canAccessContent(const char* contentId, GameRoute requiredRoute) const;
    bool isRouteAvailable(GameRoute route) const;  // Can player still reach this route?
    
    // Route progress tracking
    uint8_t getCompletedArcs() const { return m_completedArcs; }
    void markArcComplete(uint8_t arcId);
    bool isArcComplete(uint8_t arcId) const;
    uint8_t getCurrentArc() const { return m_currentArc; }
    void setCurrentArc(uint8_t arcId) { m_currentArc = arcId; }
    
    // Route hints (for UI display)
    const char* getRouteHint(GameRoute route) const;
    uint8_t getRouteProgress(GameRoute route) const;  // 0-100% progress visualization
    
    // Serialization for save system
    uint16_t getSaveSize() const;
    void serialize(uint8_t* buffer) const;
    void deserialize(const uint8_t* buffer);

private:
    RouteManager() : m_currentRoute(GameRoute::RESONANCE), m_lockedRoute(GameRoute::RESONANCE),
                     m_currentArc(0), m_completedArcs(0) {}
    ~RouteManager() {}
    RouteManager(const RouteManager&) = delete;
    RouteManager& operator=(const RouteManager&) = delete;
    
    // Normalize alignment values to -100..+100 range
    void clampAlignment(int8_t& value);
    
    // Calculate route lock state based on decisions
    void checkRouteLocks();
    
    RouteAlignment m_alignment;
    GameRoute m_currentRoute;
    GameRoute m_lockedRoute;  // ROUTE_RESONANCE if not locked
    
    RouteFlag m_flags[MAX_ROUTE_FLAGS];
    uint8_t m_flagCount;
    
    RouteEvent m_events[MAX_ROUTE_EVENTS];
    uint8_t m_eventCount;
    
    RouteDecision m_decisions[MAX_ROUTE_DECISIONS];
    uint8_t m_decisionCount;
    
    uint8_t m_currentArc;
    uint8_t m_completedArcs;
    
    // Route lock states
    RouteLockState m_routeLockStates[3];  // Index by GameRoute enum
};

#endif // ROUTE_MANAGER_H