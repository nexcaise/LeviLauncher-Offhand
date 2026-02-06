#pragma once

#include "world/item/Item.hpp"
#include "util/SharedPtr.hpp"
#include <cstddef>
#include <cstdint>

using byte = uint8_t;

class ItemStackBase {
public:
    WeakPtr<Item> mItem;
    byte mCount;
    bool mValid;
    
    bool isNull() const;
    Item* getItem() const;
    
    WeakPtr<BlockLegacy> getLegacyBlock() const;
    bool hasTag(const HashedString& tag) const;
    bool isInstance(const HashedString& itemName, bool useItemLookup) const;
};