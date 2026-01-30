/// @symbolgeneration
#pragma once
#include <unordered_map>
#include "SharedPtr.hpp"

class Item;

class ItemRegistry {
public:
    std::unordered_map<int32_t, WeakPtr<Item>> mIdToItemMap;
};