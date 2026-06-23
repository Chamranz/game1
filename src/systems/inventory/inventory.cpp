/*
 * NEON REQUIEM - Inventory System Implementation
 * Manages items, key items, data fragments, and equipment
 */

#include "inventory.h"
#include "../../engine/player.h"

// Singleton instance
Inventory& Inventory::getInstance() {
    static Inventory instance;
    return instance;
}

void Inventory::begin() {
    reset();
    
    // Initialize equipment slots
    for (int i = 0; i < MAX_EQUIPMENT; i++) {
        m_equipment[i] = nullptr;
    }
    
    LOG_INFO("Inventory: Initialized (%d item slots, %d fragment slots)", 
             MAX_ITEMS, MAX_DATA_FRAGMENTS);
}

void Inventory::reset() {
    m_itemCount = 0;
    m_registeredItemCount = 0;
    m_fragmentCount = 0;
    
    for (int i = 0; i < MAX_ITEMS; i++) {
        m_items[i].data = nullptr;
        m_items[i].quantity = 0;
        m_items[i].equipped = false;
    }
    
    for (int i = 0; i < MAX_EQUIPMENT; i++) {
        m_equipment[i] = nullptr;
    }
}

// ===== Item Database =====

void Inventory::registerItem(const ItemData& item) {
    if (m_registeredItemCount >= MAX_ITEMS) {
        LOG_WARN("Inventory: Item database full");
        return;
    }
    
    // Check for duplicate
    for (int i = 0; i < m_registeredItemCount; i++) {
        if (strcmp(m_itemDatabase[i].id, item.id) == 0) {
            LOG_VERBOSE("Inventory: Item '%s' already registered", item.id);
            return;
        }
    }
    
    m_itemDatabase[m_registeredItemCount] = item;
    m_registeredItemCount++;
}

ItemData* Inventory::findItem(const char* itemId) {
    for (int i = 0; i < m_registeredItemCount; i++) {
        if (strcmp(m_itemDatabase[i].id, itemId) == 0) {
            return &m_itemDatabase[i];
        }
    }
    return nullptr;
}

// ===== Item Management =====

bool Inventory::addItem(const ItemData& item, uint8_t quantity) {
    // Check if item already exists in inventory
    int8_t existingIdx = findItemIndex(item.id);
    
    if (existingIdx >= 0) {
        // Stack item
        uint16_t newQty = m_items[existingIdx].quantity + quantity;
        if (newQty > MAX_ITEM_STACK) {
            LOG_WARN("Inventory: Cannot stack more '%s' (max %d)", item.id, MAX_ITEM_STACK);
            return false;
        }
        m_items[existingIdx].quantity = newQty;
        LOG_VERBOSE("Inventory: Stacked '%s' x%d", item.id, m_items[existingIdx].quantity);
        return true;
    }
    
    // Need to add new item
    if (m_itemCount >= MAX_ITEMS) {
        LOG_WARN("Inventory: Full! Cannot add '%s'", item.id);
        return false;
    }
    
    // Find or register item data
    ItemData* itemData = findItem(item.id);
    if (!itemData) {
        // Register on the fly
        registerItem(item);
        itemData = findItem(item.id);
        if (!itemData) return false;
    }
    
    m_items[m_itemCount].data = itemData;
    m_items[m_itemCount].quantity = quantity;
    m_items[m_itemCount].equipped = false;
    m_itemCount++;
    
    LOG_VERBOSE("Inventory: Added '%s' x%d", item.id, quantity);
    return true;
}

bool Inventory::removeItem(const char* itemId, uint8_t quantity) {
    int8_t idx = findItemIndex(itemId);
    if (idx < 0) {
        LOG_WARN("Inventory: Item '%s' not found", itemId);
        return false;
    }
    
    if (m_items[idx].quantity < quantity) {
        LOG_WARN("Inventory: Not enough '%s' (have %d, need %d)", 
                 itemId, m_items[idx].quantity, quantity);
        return false;
    }
    
    m_items[idx].quantity -= quantity;
    
    if (m_items[idx].quantity == 0) {
        // Remove item slot
        if (m_items[idx].equipped) {
            // Unequip first
            for (int i = 0; i < MAX_EQUIPMENT; i++) {
                if (m_equipment[i] == &m_items[idx]) {
                    m_equipment[i] = nullptr;
                    break;
                }
            }
        }
        
        // Shift remaining items
        for (int i = idx; i < m_itemCount - 1; i++) {
            m_items[i] = m_items[i + 1];
        }
        m_itemCount--;
    }
    
    LOG_VERBOSE("Inventory: Removed '%s' x%d", itemId, quantity);
    return true;
}

bool Inventory::hasItem(const char* itemId) const {
    return findItemIndex(itemId) >= 0;
}

uint8_t Inventory::getItemQuantity(const char* itemId) const {
    int8_t idx = findItemIndex(itemId);
    if (idx < 0) return 0;
    return m_items[idx].quantity;
}

InventoryItem* Inventory::getItem(uint8_t index) {
    if (index >= m_itemCount) return nullptr;
    return &m_items[index];
}

// ===== Equipment =====

bool Inventory::equipItem(const char* itemId, EquipSlot slot) {
    if (slot >= MAX_EQUIPMENT) {
        LOG_WARN("Inventory: Invalid equipment slot %d", slot);
        return false;
    }
    
    int8_t idx = findItemIndex(itemId);
    if (idx < 0) {
        LOG_WARN("Inventory: Cannot equip '%s' - not in inventory", itemId);
        return false;
    }
    
    // Unequip current item in slot
    if (m_equipment[slot] != nullptr) {
        m_equipment[slot]->equipped = false;
    }
    
    // Unequip from any other slot
    for (int i = 0; i < MAX_EQUIPMENT; i++) {
        if (m_equipment[i] == &m_items[idx]) {
            m_equipment[i] = nullptr;
            break;
        }
    }
    
    // Equip
    m_items[idx].equipped = true;
    m_equipment[slot] = &m_items[idx];
    
    LOG_VERBOSE("Inventory: Equipped '%s' in slot %d", itemId, slot);
    return true;
}

bool Inventory::unequipSlot(EquipSlot slot) {
    if (slot >= MAX_EQUIPMENT) return false;
    if (m_equipment[slot] == nullptr) return false;
    
    m_equipment[slot]->equipped = false;
    m_equipment[slot] = nullptr;
    
    LOG_VERBOSE("Inventory: Unequipped slot %d", slot);
    return true;
}

InventoryItem* Inventory::getEquippedItem(EquipSlot slot) {
    if (slot >= MAX_EQUIPMENT) return nullptr;
    return m_equipment[slot];
}

bool Inventory::isSlotFilled(EquipSlot slot) const {
    if (slot >= MAX_EQUIPMENT) return false;
    return m_equipment[slot] != nullptr;
}

// ===== Item Usage =====

bool Inventory::useItem(const char* itemId, bool inBattle) {
    int8_t idx = findItemIndex(itemId);
    if (idx < 0) return false;
    
    return useItemOnIndex(idx, inBattle);
}

bool Inventory::useItemOnIndex(uint8_t index, bool inBattle) {
    if (index >= m_itemCount) return false;
    
    InventoryItem& item = m_items[index];
    if (!item.data) return false;
    
    // Check if usable in current context
    if (inBattle && !item.data->usableInBattle) return false;
    if (!inBattle && !item.data->usableInOverworld) return false;
    
    // Apply effect
    Player& player = Player::getInstance();
    
    switch (item.data->effect) {
        case EFFECT_HEAL_HP:
            player.heal(item.data->effectValue);
            break;
            
        case EFFECT_HEAL_ENERGY:
            player.addResonance(item.data->effectValue);
            break;
            
        case EFFECT_BUFF_ATTACK:
            player.setStatus(PlayerStatus::BOOSTED, true);
            break;
            
        case EFFECT_BUFF_DEFENSE:
            // Defense buff - would be implemented in battle system
            break;
            
        case EFFECT_CURE_STATUS:
            player.setStatus(PlayerStatus::GLITCHED, false);
            player.setStatus(PlayerStatus::CORRUPTED, false);
            player.setStatus(PlayerStatus::TRACED, false);
            break;
            
        case EFFECT_REVEAL_PATH:
            // Would trigger Resonance system reveal
            break;
            
        default:
            break;
    }
    
    // Consume if applicable
    if (item.data->consumable) {
        removeItem(item.data->id, 1);
    }
    
    LOG_VERBOSE("Inventory: Used '%s'", item.data->id);
    return true;
}

// ===== Data Fragments =====

bool Inventory::addDataFragment(const DataFragment& fragment) {
    if (m_fragmentCount >= MAX_DATA_FRAGMENTS) {
        LOG_WARN("Inventory: Max data fragments reached");
        return false;
    }
    
    // Check for duplicate
    for (int i = 0; i < m_fragmentCount; i++) {
        if (strcmp(m_fragments[i].id, fragment.id) == 0) {
            return false; // Already exists
        }
    }
    
    m_fragments[m_fragmentCount] = fragment;
    m_fragments[m_fragmentCount].discovered = false;
    m_fragmentCount++;
    
    LOG_VERBOSE("Inventory: Added data fragment '%s'", fragment.id);
    return true;
}

bool Inventory::discoverFragment(const char* fragmentId) {
    for (int i = 0; i < m_fragmentCount; i++) {
        if (strcmp(m_fragments[i].id, fragmentId) == 0) {
            if (!m_fragments[i].discovered) {
                m_fragments[i].discovered = true;
                LOG_INFO("Inventory: Discovered data fragment '%s'", fragmentId);
            }
            return true;
        }
    }
    return false;
}

bool Inventory::hasFragment(const char* fragmentId) const {
    for (int i = 0; i < m_fragmentCount; i++) {
        if (strcmp(m_fragments[i].id, fragmentId) == 0) {
            return m_fragments[i].discovered;
        }
    }
    return false;
}

uint8_t Inventory::getDiscoveredFragmentCount() const {
    uint8_t count = 0;
    for (int i = 0; i < m_fragmentCount; i++) {
        if (m_fragments[i].discovered) count++;
    }
    return count;
}

DataFragment* Inventory::getFragment(uint8_t index) {
    if (index >= m_fragmentCount) return nullptr;
    return &m_fragments[index];
}

uint8_t Inventory::getArcFragmentCount(uint8_t arcId) const {
    uint8_t count = 0;
    for (int i = 0; i < m_fragmentCount; i++) {
        if (m_fragments[i].arcId == arcId) count++;
    }
    return count;
}

uint8_t Inventory::getArcDiscoveredCount(uint8_t arcId) const {
    uint8_t count = 0;
    for (int i = 0; i < m_fragmentCount; i++) {
        if (m_fragments[i].arcId == arcId && m_fragments[i].discovered) count++;
    }
    return count;
}

// ===== Key Items =====

bool Inventory::hasKeyItem(const char* itemId) const {
    int8_t idx = findItemIndex(itemId);
    if (idx < 0) return false;
    return m_items[idx].data->category == CAT_KEY;
}

uint8_t Inventory::getKeyItemCount() const {
    uint8_t count = 0;
    for (int i = 0; i < m_itemCount; i++) {
        if (m_items[i].data && m_items[i].data->category == CAT_KEY) {
            count++;
        }
    }
    return count;
}

// ===== Stats =====

uint16_t Inventory::getTotalItemsValue() const {
    uint16_t total = 0;
    for (int i = 0; i < m_itemCount; i++) {
        if (m_items[i].data) {
            total += m_items[i].data->sellValue * m_items[i].quantity;
        }
    }
    return total;
}

uint8_t Inventory::getUniqueItemCount() const {
    return m_itemCount;
}

uint8_t Inventory::getEquippedCount() const {
    uint8_t count = 0;
    for (int i = 0; i < MAX_EQUIPMENT; i++) {
        if (m_equipment[i] != nullptr) count++;
    }
    return count;
}

// ===== Filtering =====

uint8_t Inventory::getItemsByCategory(ItemCategory category, InventoryItem* result, uint8_t maxResults) const {
    uint8_t count = 0;
    for (int i = 0; i < m_itemCount && count < maxResults; i++) {
        if (m_items[i].data && m_items[i].data->category == category) {
            result[count] = m_items[i];
            count++;
        }
    }
    return count;
}

uint8_t Inventory::getItemsByRarity(ItemRarity rarity, InventoryItem* result, uint8_t maxResults) const {
    uint8_t count = 0;
    for (int i = 0; i < m_itemCount && count < maxResults; i++) {
        if (m_items[i].data && m_items[i].data->rarity == rarity) {
            result[count] = m_items[i];
            count++;
        }
    }
    return count;
}

// ===== Internal Helpers =====

int8_t Inventory::findItemIndex(const char* itemId) const {
    for (int i = 0; i < m_itemCount; i++) {
        if (m_items[i].data && strcmp(m_items[i].data->id, itemId) == 0) {
            return i;
        }
    }
    return -1;
}

int8_t Inventory::findRegisteredItemIndex(const char* itemId) const {
    for (int i = 0; i < m_registeredItemCount; i++) {
        if (strcmp(m_itemDatabase[i].id, itemId) == 0) {
            return i;
        }
    }
    return -1;
}

// ===== Serialization =====

uint16_t Inventory::getSaveSize() const {
    uint16_t size = sizeof(uint8_t); // itemCount
    
    for (int i = 0; i < m_itemCount; i++) {
        size += sizeof(ItemData);     // Item data
        size += sizeof(uint8_t);      // quantity
        size += sizeof(bool);         // equipped
    }
    
    size += sizeof(uint8_t);          // fragmentCount
    size += sizeof(DataFragment) * m_fragmentCount;
    
    size += sizeof(uint8_t) * MAX_EQUIPMENT; // equipment slot indices
    
    return size;
}

void Inventory::serialize(uint8_t* buffer) const {
    uint16_t offset = 0;
    
    // Save item count
    buffer[offset++] = m_itemCount;
    
    // Save items
    for (int i = 0; i < m_itemCount; i++) {
        if (m_items[i].data) {
            memcpy(buffer + offset, m_items[i].data, sizeof(ItemData));
        } else {
            memset(buffer + offset, 0, sizeof(ItemData));
        }
        offset += sizeof(ItemData);
        
        buffer[offset++] = m_items[i].quantity;
        buffer[offset++] = m_items[i].equipped ? 1 : 0;
    }
    
    // Save fragments
    buffer[offset++] = m_fragmentCount;
    memcpy(buffer + offset, m_fragments, sizeof(DataFragment) * m_fragmentCount);
    offset += sizeof(DataFragment) * m_fragmentCount;
    
    // Save equipment indices
    for (int i = 0; i < MAX_EQUIPMENT; i++) {
        if (m_equipment[i] != nullptr) {
            // Store index of equipped item
            int8_t idx = -1;
            for (int j = 0; j < m_itemCount; j++) {
                if (&m_items[j] == m_equipment[i]) {
                    idx = j;
                    break;
                }
            }
            buffer[offset++] = (uint8_t)idx;
        } else {
            buffer[offset++] = 0xFF; // Empty slot
        }
    }
}

void Inventory::deserialize(const uint8_t* buffer) {
    uint16_t offset = 0;
    
    // Clear current state
    reset();
    
    // Load item count
    m_itemCount = buffer[offset++];
    m_itemCount = min(m_itemCount, (uint8_t)MAX_ITEMS);
    
    // Load items
    for (int i = 0; i < m_itemCount; i++) {
        // Allocate item data from registered items or create temporary
        ItemData tempData;
        memcpy(&tempData, buffer + offset, sizeof(ItemData));
        offset += sizeof(ItemData);
        
        // Find or register
        ItemData* itemData = findItem(tempData.id);
        if (!itemData) {
            registerItem(tempData);
            itemData = findItem(tempData.id);
        }
        
        m_items[i].data = itemData;
        m_items[i].quantity = buffer[offset++];
        m_items[i].equipped = buffer[offset++] != 0;
    }
    
    // Load fragments
    m_fragmentCount = buffer[offset++];
    m_fragmentCount = min(m_fragmentCount, (uint8_t)MAX_DATA_FRAGMENTS);
    memcpy(m_fragments, buffer + offset, sizeof(DataFragment) * m_fragmentCount);
    offset += sizeof(DataFragment) * m_fragmentCount;
    
    // Load equipment
    for (int i = 0; i < MAX_EQUIPMENT; i++) {
        uint8_t idx = buffer[offset++];
        if (idx < m_itemCount) {
            m_equipment[i] = &m_items[idx];
        } else {
            m_equipment[i] = nullptr;
        }
    }
    
    LOG_INFO("Inventory: Restored (%d items, %d fragments, %d equipped)", 
             m_itemCount, m_fragmentCount, getEquippedCount());
}