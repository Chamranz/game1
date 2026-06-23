// ============================================================================
// NEON REQUIEM — Battle Pattern System (Implementation)
// ============================================================================

#include "battle_pattern.h"
#include <cstdlib>

BattlePattern::BattlePattern()
    : m_length(4), m_inputPos(0), m_difficulty(1), m_inputting(false) {
    for (int i = 0; i < PATTERN_MAX_LENGTH; i++) {
        m_pattern[i] = PatternDir::UP;
    }
}

void BattlePattern::generatePattern(int length) {
    m_length = min(length, PATTERN_MAX_LENGTH);
    for (int i = 0; i < m_length; i++) {
        m_pattern[i] = static_cast<PatternDir>(rand() % 6);
    }
}

void BattlePattern::startInput() {
    m_inputPos = 0;
    m_inputting = true;
}

int BattlePattern::processInput(PatternDir dir) {
    if (!m_inputting) return 0;

    if (dir == m_pattern[m_inputPos]) {
        m_inputPos++;
        if (m_inputPos >= m_length) {
            m_inputting = false;
            return 1;  // Успех
        }
        return 0;  // Продолжаем
    }

    m_inputting = false;
    return -1;  // Ошибка
}