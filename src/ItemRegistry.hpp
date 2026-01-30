/// @symbolgeneration
#pragma once
#include <unordered_map>
#include "mc/src/common/SharedPtr.hpp"

class ItemRegistry {
public:
    std::unordered_map<int32_t, WeakPtr<Item>> mIdToItemMap;
};