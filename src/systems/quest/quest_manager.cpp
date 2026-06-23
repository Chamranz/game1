// ============================================================================
// NEON REQUIEM — Quest Manager (Implementation)
// ============================================================================

#include "quest_manager.h"

QuestManager::QuestManager() : m_questCount(0) {}

void QuestManager::begin() { m_questCount = 0; }

void QuestManager::update() {
    // Проверка условий завершения квестов
}

bool QuestManager::addQuest(uint16_t questId) {
    if (m_questCount >= MAX_ACTIVE_QUESTS) return false;

    // TODO: загрузка данных квеста из JSON
    m_quests[m_questCount].id = questId;
    m_quests[m_questCount].status = QuestStatus::ACTIVE;
    m_questCount++;
    return true;
}

bool QuestManager::completeQuest(uint16_t questId) {
    for (int i = 0; i < m_questCount; i++) {
        if (m_quests[i].id == questId) {
            m_quests[i].status = QuestStatus::COMPLETE;
            return true;
        }
    }
    return false;
}

bool QuestManager::failQuest(uint16_t questId) {
    for (int i = 0; i < m_questCount; i++) {
        if (m_quests[i].id == questId) {
            m_quests[i].status = QuestStatus::FAILED;
            return true;
        }
    }
    return false;
}

bool QuestManager::advanceObjective(uint16_t questId, uint8_t objectiveIndex) {
    for (int i = 0; i < m_questCount; i++) {
        if (m_quests[i].id == questId && objectiveIndex < m_quests[i].objectiveCount) {
            m_quests[i].objectiveProgress[objectiveIndex]++;
            return true;
        }
    }
    return false;
}

QuestData* QuestManager::getQuest(uint16_t questId) {
    for (int i = 0; i < m_questCount; i++) {
        if (m_quests[i].id == questId) return &m_quests[i];
    }
    return nullptr;
}

int QuestManager::getActiveQuestCount() const {
    int count = 0;
    for (int i = 0; i < m_questCount; i++) {
        if (m_quests[i].status == QuestStatus::ACTIVE) count++;
    }
    return count;
}

QuestData* QuestManager::getActiveQuests() { return m_quests; }

bool QuestManager::isQuestComplete(uint16_t questId) const {
    for (int i = 0; i < m_questCount; i++) {
        if (m_quests[i].id == questId) return m_quests[i].status == QuestStatus::COMPLETE;
    }
    return false;
}

bool QuestManager::isQuestActive(uint16_t questId) const {
    for (int i = 0; i < m_questCount; i++) {
        if (m_quests[i].id == questId) return m_quests[i].status == QuestStatus::ACTIVE;
    }
    return false;
}