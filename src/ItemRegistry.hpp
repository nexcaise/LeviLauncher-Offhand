/// @symbolgeneration
#pragma once
#include <unordered_map>
#include "SharedPtr.hpp"
#include "Item.hpp"

class ItemRegistry {
public:
    std::unordered_map<int32_t, WeakPtr<Item>> mIdToItemMap;
};