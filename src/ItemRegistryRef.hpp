#pragma once
#include <memory>
#include <unordered_map>
#include "ItemRegistry.hpp"

class ItemRegistryRef {
public:
    std::weak_ptr<ItemRegistry> mWeakRegistry;
    std::weak_ptr<ItemRegistry> mItemRegistry;

    inline std::unordered_map<StringHash, WeakPtr<Item>> const& getNameToItemMap() const {
        return _lockRegistry()->mNameToItemMap;
    }

    inline std::shared_ptr<ItemRegistry> _lockRegistry() const {
        auto sp = mWeakRegistry.lock();
        if (!sp) sp = mItemRegistry.lock();
        return sp;
    }
};