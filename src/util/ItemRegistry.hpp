#pragma once
#include <unordered_map>
#include "util/SharedPtr.hpp"
#include "util/Item.hpp"
#include "util/string/StringHash.hpp"

class ItemRegistry {
public:
    std::unordered_map<int, WeakPtr<Item>> mIdToItemMap;
    
    std::unordered_map<HashedString, WeakPtr<Item>> mNameToItemMap;
};