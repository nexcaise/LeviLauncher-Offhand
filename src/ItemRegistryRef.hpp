#pragma once
#include <memory>
#include <unordered_map>
#include "SharedPtr.hpp"
#include "Item.hpp"
//#include "StringHash.hpp"

class ItemRegistry;

class StringHash {
public:
    int mHash;
};

class ItemRegistryRef {
public:
    std::unordered_map<StringHash, WeakPtr<Item>> nameToItemMap;
    std::weak_ptr<ItemRegistry> mWeakRegistry;
    std::weak_ptr<ItemRegistry> mItemRegistry;
    
    std::unordered_map<StringHash, WeakPtr<Item>> const& getNameToItemMap() const;

    std::shared_ptr<ItemRegistry> _lockRegistry() const;
};
