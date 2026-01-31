/// @symbolgeneration
#pragma once
#include <unordered_map>
#include "SharedPtr.hpp"
#include "Item.hpp"
#include <vector>

class StringHash {
public:
    int mHash;
    bool operator==(StringHash const& o) const { return mHash == o.mHash; }
};

class ItemRegistry {
public:
    std::vector<SharedPtr<Item>> mItemRegistry;
    std::unordered_map<int, WeakPtr<Item>> mIdToItemMap;
    std::unordered_map<StringHash, WeakPtr<Item>> mNameToItemMap;
};