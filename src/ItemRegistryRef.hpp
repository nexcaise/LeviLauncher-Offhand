#pragma once
#include <memory>
#include <unordered_map>
#include "ItemRegistry.hpp"

class ItemRegistryRef {
public:
    std::weak_ptr<ItemRegistry> mWeakRegistry;

    inline std::shared_ptr<ItemRegistry> _lockRegistry() const {
        auto sp = mWeakRegistry.lock();
        return sp;
    }
};