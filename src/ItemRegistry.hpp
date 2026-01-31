#pragma once
#include <unordered_map>
#include <vector>
#include "SharedPtr.hpp"
#include "Item.hpp"
#include "StringHash.hpp"

class ItemRegistry {
public:
    //std::vector<SharedPtr<Item>> mItemRegistry;
    std::unordered_map<int, WeakPtr<Item>> mIdToItemMap;
};