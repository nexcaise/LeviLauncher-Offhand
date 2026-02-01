#pragma once
#include <memory>
#include <unordered_map>
#include "ItemRegistry.hpp"

class ItemRegistryRef {
public:
    std::weak_ptr<ItemRegistry> mWeakRegistry;
    
    inline int getItemCount() {
        return 1;
    } const;
};