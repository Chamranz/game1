// ============================================================================
// NEON REQUIEM — Battle Pattern System (Header)
// ============================================================================
// PATTERN-бои: игрок запоминает и повторяет последовательность атак.
// ============================================================================

#ifndef NEON_REQUIEM_BATTLE_PATTERN_H
#define NEON_REQUIEM_BATTLE_PATTERN_H

#include <cstdint>
#include <Arduino.h>
#include "config/config.h"

/// Максимальная длина паттерна
constexpr int PATTERN_MAX_LENGTH = 8;

/// Направление в паттерне
enum class PatternDir : uint8_t {
    UP = 0, DOWN = 1, LEFT = 2, RIGHT = 3, A = 4, B = 5
};

class BattlePattern {
public:
    BattlePattern();

    /// Сгенерировать новый паттерн
    void generatePattern(int length);

    /// Начать ввод паттерна
    void startInput();

    /// Обработать ввод
    /// @return 0=продолжаем, 1=успех, -1=ошибка
    int processInput(PatternDir dir);

    /// Получить текущий паттерн для отображения
    const PatternDir* getPattern() const { return m_pattern; }

    /// Получить длину паттерна
    int getLength() const { return m_length; }

    /// Получить текущую позицию ввода
    int getInputPosition() const { return m_inputPos; }

    /// Получить сложность (0-5)
    int getDifficulty() const { return m_difficulty; }

    /// Установить сложность
    void setDifficulty(int diff) { m_difficulty = diff; }

private:
    PatternDir m_pattern[PATTERN_MAX_LENGTH];
    int m_length;
    int m_inputPos;
    int m_difficulty;
    bool m_inputting;
};

#endif