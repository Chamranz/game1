/*
 * NEON REQUIEM - Inventory System
 * Manages items, key items, data fragments, and equipment
 * Items affect battle stats, dialog options, and route access
 */

#ifndef INVENTORY_H
#define INVENTORY_H

#include <Arduino.h>
#include "config/config.h"

// Inventory limits
#define MAX_ITEMS 32
#define MAX_KEY_ITEMS 24
#define MAX_DATA_FRAGMENTS 16
#define MAX_EQUIPMENT 8
#define MAX_ITEM_STACK 99

// Item categories
enum ItemCategory : uint8_t {
    CAT_CONSUMABLE = 0,    // Healing, buff items
    CAT_WEAPON = 1,        // HACK program upgrades
    CAT_ARMOR = 2,         // Defense upgrades
    CAT_DATA = 3,          // Lore fragments, intel
    CAT_KEY = 4,           // Story progression items
    CAT_CYBERWARE = 5,     // Permanent upgrades
    CAT_CRAFTING = 6       // Crafting components
};

// Item rarity
enum ItemRarity : uint8_t {
    RARITY_COMMON = 0,
    RARITY_UNCOMMON = 1,
    RARITY_RARE = 2,
    RARITY_LEGENDARY = 3
};

// Item effects (applied on use)
enum ItemEffect : uint8_t {
    EFFECT_NONE = 0,
    EFFECT_HEAL_HP = 1,        // Restore HP
    EFFECT_HEAL_ENERGY = 2,    // Restore RESONANCE energy
    EFFECT_BUFF_ATTACK = 3,    // Temporary attack up
    EFFECT_BUFF_DEFENSE = 4,   // Temporary defense up
    EFFECT_BUFF_SPEED = 5,     // Temporary speed up
    EFFECT_CURE_STATUS = 6,    // Remove status effects
    EFFECT_REVEAL_PATH = 7,    // Reveal hidden paths (Resonance)
    EFFECT_DATA_BREACH = 8,    // Unlock data fragment
    EFFECT_QUEST_ITEM = 9,     // Quest progression
    EFFECT_ROUTE_KEY = 10      // Route-locking key item
};

// Item data structure
struct ItemData {
    char id[20];               // Item identifier
    char name[24];             // Display name
    char description[64];      // Item description
    ItemCategory category;     // Item category
    ItemRarity rarity;         // Item rarity
    ItemEffect effect;         // Use effect
    int8_t effectValue;        // Effect magnitude
    uint8_t sellValue;         // Value in credits
    bool consumable;           // Removed after use?
    bool usableInBattle;       // Can use in combat?
    bool usableInOverworld;    // Can use on map?
    uint8_t routeRequirement;  // 0=any, 1=RESONANCE, 2=ERASE, 3=GLITCH
};

// Inventory item instance
struct InventoryItem {
    ItemData* data;            // Pointer to item data
    uint8_t quantity;          // Stack quantity
    bool equipped;             // Currently equipped?
};

// Equipment slots
enum EquipSlot : uint8_t {
    EQUIP_HACK_PROGRAM = 0,    // Primary attack program
    EQUIP_DEFENSE_SYS = 1,     // Defense subsystem
    EQUIP_CYBERWARE_1 = 2,    // Cyberware slot 1
    EQUIP_CYBERWARE_2 = 3,    // Cyberware slot 2
    EQUIP_CYBERWARE_3 = 4,    // Cyberware slot 3
    EQUIP_ARMOR = 5,           // Body armor
    EQUIP_ACCESSORY = 6,       // Accessory slot
    EQUIP_DATA_CORE = 7        // Data core (story item)
};

// Data fragment (lore collectible)
struct DataFragment {
    char id[20];               // Fragment identifier
    char title[32];            // Fragment title
    char content[128];         // Fragment content
    uint8_t arcId;             // Which arc it belongs to
    bool discovered;           // Has been found?
    uint8_t fragmentIndex;     // Position in sequence
};

class Inventory {
public:
    // Singleton access
    static Inventory& getInstance();
    
    // Initialization
    void begin();
    void reset();
    
    // Item management
    bool addItem(const ItemData& item, uint8_t quantity = 1);
    bool removeItem(const char* itemId, uint8_t quantity = 1);
    bool hasItem(const char* itemId) const;
    uint8_t getItemQuantity(const char* itemId) const;
    InventoryItem* getItem(uint8_t index);
    uint8_t getItemCount() const { return m_itemCount; }
    
    // Item database
    void registerItem(const ItemData& item);
    ItemData* findItem(const char* itemId);
    uint8_t getRegisteredItemCount() const { return m_registeredItemCount; }
    
    // Equipment
    bool equipItem(const char* itemId, EquipSlot slot);
    bool unequipSlot(EquipSlot slot);
    InventoryItem* getEquippedItem(EquipSlot slot);
    bool isSlotFilled(EquipSlot slot) const;
    
    // Item usage
    bool useItem(const char* itemId, bool inBattle);
    bool useItemOnIndex(uint8_t index, bool inBattle);
    
    // Data fragments
    bool addDataFragment(const DataFragment& fragment);
    bool discoverFragment(const char* fragmentId);
    bool hasFragment(const char* fragmentId) const;
    uint8_t getFragmentCount() const { return m_fragmentCount; }
    uint8_t getDiscoveredFragmentCount() const;
    DataFragment* getFragment(uint8_t index);
    uint8_t getArcFragmentCount(uint8_t arcId) const;
    uint8_t getArcDiscoveredCount(uint8_t arcId) const;
    
    // Key items (story progression)
    bool hasKeyItem(const char* itemId) const;
    uint8_t getKeyItemCount() const;
    
    // Stats
    uint16_t getTotalItemsValue() const;
    uint8_t getUniqueItemCount() const;
    uint8_t getEquippedCount() const;
    
    // Filtering
    uint8_t getItemsByCategory(ItemCategory category, InventoryItem* result, uint8_t maxResults) const;
    uint8_t getItemsByRarity(ItemRarity rarity, InventoryItem* result, uint8_t maxResults) const;
    
    // Serialization for save system
    uint16_t getSaveSize() const;
    void serialize(uint8_t* buffer) const;
    void deserialize(const uint8_t* buffer);

private:
    Inventory() : m_itemCount(0), m_registeredItemCount(0), m_fragmentCount(0) {}
    ~Inventory() {}
    Inventory(const Inventory&) = delete;
    Inventory& operator=(const Inventory&) = delete;
    
    // Find item index in inventory
    int8_t findItemIndex(const char* itemId) const;
    int8_t findRegisteredItemIndex(const char* itemId) const;
    
    // Item database (templates)
    ItemData m_itemDatabase[MAX_ITEMS];
    uint8_t m_registeredItemCount;
    
    // Player inventory
    InventoryItem m_items[MAX_ITEMS];
    uint8_t m_itemCount;
    
    // Equipment
    InventoryItem* m_equipment[MAX_EQUIPMENT];
    
    // Data fragments
    DataFragment m_fragments[MAX_DATA_FRAGMENTS];
    uint8_t m_fragmentCount;
};

#endif // INVENTORY_H