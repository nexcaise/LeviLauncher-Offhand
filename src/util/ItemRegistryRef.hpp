#pragma once
#include <memory>
#include <unordered_map>
#include "util/ItemRegistry.hpp"

class ItemRegistryRef {
public:
    std::weak_ptr<ItemRegistry> mWeakRegistry;
    
    std::shared_ptr<ItemRegistry> _lockRegistry() const;
};