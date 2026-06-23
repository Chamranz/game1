/*
 * NEON REQUIEM - Route Manager Implementation
 * Handles game route tracking, branching logic, and route-specific content
 */

#include "route_manager.h"
#include "config/palette.h"

// Singleton instance
RouteManager& RouteManager::getInstance() {
    static RouteManager instance;
    return instance;
}

void RouteManager::begin() {
    reset();
    LOG_INFO("RouteManager: Initialized with 3 routes (RESONANCE, ERASE, GLITCH)");
}

void RouteManager::reset() {
    m_alignment.resonance = 0;
    m_alignment.erase = 0;
    m_alignment.glitch = 0;
    m_currentRoute = GameRoute::RESONANCE;
    m_lockedRoute = GameRoute::RESONANCE;
    m_currentArc = 0;
    m_completedArcs = 0;
    m_flagCount = 0;
    m_eventCount = 0;
    m_decisionCount = 0;
    
    for (int i = 0; i < 3; i++) {
        m_routeLockStates[i] = ROUTE_UNLOCKED;
    }
    
    LOG_INFO("RouteManager: Reset complete");
}

// ===== Route Alignment Management =====

void RouteManager::addDecisionImpact(const char* decisionId, DecisionImpact impact, uint8_t arcId, bool critical) {
    if (m_decisionCount >= MAX_ROUTE_DECISIONS) {
        LOG_WARN("RouteManager: Max decisions reached");
        return;
    }
    
    // Store the decision
    RouteDecision& decision = m_decisions[m_decisionCount];
    strncpy(decision.id, decisionId, sizeof(decision.id) - 1);
    decision.id[sizeof(decision.id) - 1] = '\0';
    decision.impact = impact;
    decision.arcId = arcId;
    decision.timestamp = millis() & 0xFFFF;
    decision.critical = critical;
    m_decisionCount++;
    
    // Calculate impact value (critical decisions count double)
    int8_t impactValue = critical ? 15 : 8;
    
    // Apply impact to route alignment
    switch (impact) {
        case IMPACT_RESONANCE_POS:
            m_alignment.resonance = constrain(m_alignment.resonance + impactValue, -100, 100);
            break;
        case IMPACT_RESONANCE_NEG:
            m_alignment.resonance = constrain(m_alignment.resonance - impactValue, -100, 100);
            break;
        case IMPACT_ERASE_POS:
            m_alignment.erase = constrain(m_alignment.erase + impactValue, -100, 100);
            break;
        case IMPACT_ERASE_NEG:
            m_alignment.erase = constrain(m_alignment.erase - impactValue, -100, 100);
            break;
        case IMPACT_GLITCH_POS:
            m_alignment.glitch = constrain(m_alignment.glitch + impactValue, -100, 100);
            break;
        case IMPACT_GLITCH_NEG:
            m_alignment.glitch = constrain(m_alignment.glitch - impactValue, -100, 100);
            break;
        case IMPACT_CRITICAL:
            // Critical decisions affect all routes
            m_alignment.resonance = constrain(m_alignment.resonance + impactValue, -100, 100);
            m_alignment.erase = constrain(m_alignment.erase - impactValue / 2, -100, 100);
            m_alignment.glitch = constrain(m_alignment.glitch - impactValue / 2, -100, 100);
            break;
        default:
            break;
    }
    
    // Check if any route should be locked
    checkRouteLocks();
    
    LOG_VERBOSE("RouteManager: Decision '%s' impact=%d, alignment R:%d E:%d G:%d",
                decisionId, impact, m_alignment.resonance, m_alignment.erase, m_alignment.glitch);
}

int8_t RouteManager::getRouteAlignment(GameRoute route) const {
    switch (route) {
        case GameRoute::RESONANCE: return m_alignment.resonance;
        case GameRoute::ERASE:     return m_alignment.erase;
        case GameRoute::GLITCH:    return m_alignment.glitch;
        default:              return 0;
    }
}

// ===== Route Locking =====

bool RouteManager::isRouteLocked(GameRoute route) const {
    if (route == GameRoute::RESONANCE) return false; // Default route is never locked
    return m_routeLockStates[(int)route] == ROUTE_LOCKED_IN || 
           m_routeLockStates[(int)route] == ROUTE_LOCKED_OUT;
}

RouteLockState RouteManager::getRouteLockState(GameRoute route) const {
    if (route == GameRoute::RESONANCE) return ROUTE_UNLOCKED;
    return m_routeLockStates[(int)route];
}

bool RouteManager::lockRoute(GameRoute route) {
    if (route == GameRoute::RESONANCE) {
        // Can't force-lock the default route
        return false;
    }
    
    m_routeLockStates[(int)route] = ROUTE_LOCKED_IN;
    m_lockedRoute = route;
    m_currentRoute = route;
    
    LOG_INFO("RouteManager: Route %d locked in!", route);
    return true;
}

void RouteManager::checkRouteLocks() {
    // Check if any non-default route has enough alignment to lock in
    for (int route = 1; route <= 2; route++) {
        if (m_routeLockStates[route] != ROUTE_UNLOCKED) continue;
        
        int8_t alignment = (route == (int)GameRoute::ERASE) ? m_alignment.erase : m_alignment.glitch;
        
        // Lock in if alignment exceeds threshold
        if (alignment >= ROUTE_LOCK_THRESHOLD * 10) {
            m_routeLockStates[route] = ROUTE_LOCKED_IN;
            m_lockedRoute = (GameRoute)route;
            m_currentRoute = (GameRoute)route;
            
            // Lock out the other non-default route
            int otherRoute = (route == (int)GameRoute::ERASE) ? (int)GameRoute::GLITCH : (int)GameRoute::ERASE;
            m_routeLockStates[otherRoute] = ROUTE_LOCKED_OUT;
            
            LOG_INFO("RouteManager: Route %d locked in! Route %d locked out.", route, otherRoute);
        }
        
        // Lock out if negative alignment exceeds threshold
        if (alignment <= -(ROUTE_LOCK_THRESHOLD * 10)) {
            m_routeLockStates[route] = ROUTE_LOCKED_OUT;
            LOG_INFO("RouteManager: Route %d locked out (negative alignment).", route);
        }
    }
}

// ===== Route Determination =====

GameRoute RouteManager::determineRoute() const {
    // If a route is locked in, return it
    if (m_lockedRoute != GameRoute::RESONANCE) {
        return m_lockedRoute;
    }
    
    // Check lock states
    for (int i = 1; i <= 2; i++) {
        if (m_routeLockStates[i] == ROUTE_LOCKED_IN) {
            return (GameRoute)i;
        }
    }
    
    // Calculate which route has the highest alignment
    int8_t scores[3] = { m_alignment.resonance, m_alignment.erase, m_alignment.glitch };
    
    // Filter out locked-out routes
    for (int i = 1; i <= 2; i++) {
        if (m_routeLockStates[i] == ROUTE_LOCKED_OUT) {
            scores[i] = -128; // Effectively disqualify
        }
    }
    
    // Find highest score
    uint8_t bestRoute = 0;
    int8_t bestScore = scores[0];
    
    for (int i = 1; i < 3; i++) {
        if (scores[i] > bestScore) {
            bestScore = scores[i];
            bestRoute = i;
        }
    }
    
    return (GameRoute)bestRoute;
}

void RouteManager::updateRoute() {
    GameRoute newRoute = determineRoute();
    if (newRoute != m_currentRoute) {
        GameRoute oldRoute = m_currentRoute;
        m_currentRoute = newRoute;
        LOG_INFO("RouteManager: Route changed from %d to %d", oldRoute, newRoute);
    }
}

// ===== Route Flags =====

void RouteManager::setFlag(const char* flagId, bool value, uint8_t arcId) {
    // Check if flag already exists
    for (int i = 0; i < m_flagCount; i++) {
        if (strcmp(m_flags[i].id, flagId) == 0) {
            m_flags[i].value = value;
            m_flags[i].arcId = arcId;
            return;
        }
    }
    
    // Create new flag
    if (m_flagCount < MAX_ROUTE_FLAGS) {
        RouteFlag& flag = m_flags[m_flagCount];
        strncpy(flag.id, flagId, sizeof(flag.id) - 1);
        flag.id[sizeof(flag.id) - 1] = '\0';
        flag.value = value;
        flag.arcId = arcId;
        m_flagCount++;
    } else {
        LOG_WARN("RouteManager: Max flags reached");
    }
}

bool RouteManager::getFlag(const char* flagId) const {
    for (int i = 0; i < m_flagCount; i++) {
        if (strcmp(m_flags[i].id, flagId) == 0) {
            return m_flags[i].value;
        }
    }
    return false; // Flag not found = false
}

void RouteManager::clearFlags(uint8_t arcId) {
    uint8_t writeIdx = 0;
    for (int i = 0; i < m_flagCount; i++) {
        if (m_flags[i].arcId != arcId) {
            if (writeIdx != i) {
                m_flags[writeIdx] = m_flags[i];
            }
            writeIdx++;
        }
    }
    m_flagCount = writeIdx;
}

void RouteManager::clearAllFlags() {
    m_flagCount = 0;
}

// ===== Route Events =====

void RouteManager::recordEvent(RouteEventType type, const char* eventId, uint8_t arcId) {
    if (m_eventCount >= MAX_ROUTE_EVENTS) {
        LOG_WARN("RouteManager: Max events reached");
        return;
    }
    
    RouteEvent& event = m_events[m_eventCount];
    event.type = type;
    strncpy(event.id, eventId, sizeof(event.id) - 1);
    event.id[sizeof(event.id) - 1] = '\0';
    event.arcId = arcId;
    event.timestamp = millis() & 0xFFFF;
    m_eventCount++;
}

bool RouteManager::hasEventOccurred(const char* eventId) const {
    for (int i = 0; i < m_eventCount; i++) {
        if (strcmp(m_events[i].id, eventId) == 0) {
            return true;
        }
    }
    return false;
}

int RouteManager::getEventCount(RouteEventType type) const {
    int count = 0;
    for (int i = 0; i < m_eventCount; i++) {
        if (m_events[i].type == type) count++;
    }
    return count;
}

// ===== Route-Specific Content Access =====

bool RouteManager::canAccessContent(const char* contentId, GameRoute requiredRoute) const {
    // If no route is locked, all content is accessible
    if (m_lockedRoute == GameRoute::RESONANCE) {
        // Check if any route is locked out
        for (int i = 1; i <= 2; i++) {
            if (m_routeLockStates[i] == ROUTE_LOCKED_OUT && i == (int)requiredRoute) {
                return false;
            }
        }
        return true;
    }
    
    // If a route is locked, only that route's content is accessible
    return m_lockedRoute == requiredRoute;
}

bool RouteManager::isRouteAvailable(GameRoute route) const {
    if (route == GameRoute::RESONANCE) return true; // Always available
    
    RouteLockState state = m_routeLockStates[(int)route];
    return state == ROUTE_UNLOCKED;
}

// ===== Route Progress Tracking =====

void RouteManager::markArcComplete(uint8_t arcId) {
    if (arcId < 5) {
        m_completedArcs |= (1 << arcId);
        LOG_INFO("RouteManager: Arc %d completed", arcId);
    }
}

bool RouteManager::isArcComplete(uint8_t arcId) const {
    if (arcId >= 5) return false;
    return (m_completedArcs & (1 << arcId)) != 0;
}

// ===== Route Hints =====

const char* RouteManager::getRouteHint(GameRoute route) const {
    switch (route) {
        case GameRoute::RESONANCE:
            return "RESONANCE: Connect, understand, redeem";
        case GameRoute::ERASE:
            return "ERASE: Dominate, control, destroy";
        case GameRoute::GLITCH:
            return "GLITCH: Question, expose, transcend";
        default:
            return "UNKNOWN ROUTE";
    }
}

uint8_t RouteManager::getRouteProgress(GameRoute route) const {
    int8_t alignment = getRouteAlignment(route);
    
    // Convert alignment (-100..+100) to progress (0..100)
    // Negative alignment = 0-49%, positive = 50-100%
    if (alignment <= 0) {
        return map(alignment, -100, 0, 0, 49);
    } else {
        return map(alignment, 0, 100, 50, 100);
    }
}

// ===== Serialization =====

uint16_t RouteManager::getSaveSize() const {
    return sizeof(RouteAlignment) + 
           sizeof(GameRoute) * 2 +
           sizeof(uint8_t) * 2 +  // currentArc, completedArcs
           sizeof(RouteLockState) * 3 +
           sizeof(uint8_t) +      // flagCount
           sizeof(RouteFlag) * m_flagCount +
           sizeof(uint8_t) +      // eventCount
           sizeof(RouteEvent) * m_eventCount +
           sizeof(uint8_t) +      // decisionCount
           sizeof(RouteDecision) * m_decisionCount;
}

void RouteManager::serialize(uint8_t* buffer) const {
    uint16_t offset = 0;
    
    memcpy(buffer + offset, &m_alignment, sizeof(RouteAlignment));
    offset += sizeof(RouteAlignment);
    
    memcpy(buffer + offset, &m_currentRoute, sizeof(GameRoute));
    offset += sizeof(GameRoute);
    
    memcpy(buffer + offset, &m_lockedRoute, sizeof(GameRoute));
    offset += sizeof(GameRoute);
    
    memcpy(buffer + offset, &m_currentArc, sizeof(uint8_t));
    offset += sizeof(uint8_t);
    
    memcpy(buffer + offset, &m_completedArcs, sizeof(uint8_t));
    offset += sizeof(uint8_t);
    
    memcpy(buffer + offset, m_routeLockStates, sizeof(RouteLockState) * 3);
    offset += sizeof(RouteLockState) * 3;
    
    // Serialize flags
    buffer[offset++] = m_flagCount;
    memcpy(buffer + offset, m_flags, sizeof(RouteFlag) * m_flagCount);
    offset += sizeof(RouteFlag) * m_flagCount;
    
    // Serialize events
    buffer[offset++] = m_eventCount;
    memcpy(buffer + offset, m_events, sizeof(RouteEvent) * m_eventCount);
    offset += sizeof(RouteEvent) * m_eventCount;
    
    // Serialize decisions
    buffer[offset++] = m_decisionCount;
    memcpy(buffer + offset, m_decisions, sizeof(RouteDecision) * m_decisionCount);
}

void RouteManager::deserialize(const uint8_t* buffer) {
    uint16_t offset = 0;
    
    memcpy(&m_alignment, buffer + offset, sizeof(RouteAlignment));
    offset += sizeof(RouteAlignment);
    
    memcpy(&m_currentRoute, buffer + offset, sizeof(GameRoute));
    offset += sizeof(GameRoute);
    
    memcpy(&m_lockedRoute, buffer + offset, sizeof(GameRoute));
    offset += sizeof(GameRoute);
    
    memcpy(&m_currentArc, buffer + offset, sizeof(uint8_t));
    offset += sizeof(uint8_t);
    
    memcpy(&m_completedArcs, buffer + offset, sizeof(uint8_t));
    offset += sizeof(uint8_t);
    
    memcpy(m_routeLockStates, buffer + offset, sizeof(RouteLockState) * 3);
    offset += sizeof(RouteLockState) * 3;
    
    // Deserialize flags
    m_flagCount = buffer[offset++];
    m_flagCount = min(m_flagCount, (uint8_t)MAX_ROUTE_FLAGS);
    memcpy(m_flags, buffer + offset, sizeof(RouteFlag) * m_flagCount);
    offset += sizeof(RouteFlag) * m_flagCount;
    
    // Deserialize events
    m_eventCount = buffer[offset++];
    m_eventCount = min(m_eventCount, (uint8_t)MAX_ROUTE_EVENTS);
    memcpy(m_events, buffer + offset, sizeof(RouteEvent) * m_eventCount);
    offset += sizeof(RouteEvent) * m_eventCount;
    
    // Deserialize decisions
    m_decisionCount = buffer[offset++];
    m_decisionCount = min(m_decisionCount, (uint8_t)MAX_ROUTE_DECISIONS);
    memcpy(m_decisions, buffer + offset, sizeof(RouteDecision) * m_decisionCount);
    
    LOG_INFO("RouteManager: State restored (route=%d, flags=%d, events=%d, decisions=%d)",
             m_currentRoute, m_flagCount, m_eventCount, m_decisionCount);
}

// ===== RouteAlignment Helper =====

GameRoute RouteAlignment::getDominantRoute() const {
    int8_t scores[3] = { resonance, erase, glitch };
    uint8_t best = 0;
    for (int i = 1; i < 3; i++) {
        if (scores[i] > scores[best]) best = i;
    }
    return (GameRoute)best;
}

bool RouteAlignment::isRouteLocked(GameRoute route) const {
    // This is a simplified check - actual lock state is in RouteManager
    return false;
}

void RouteManager::clampAlignment(int8_t& value) {
    if (value < -100) value = -100;
    if (value > 100) value = 100;
}